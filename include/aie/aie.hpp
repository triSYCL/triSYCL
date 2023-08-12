
#ifndef AIE_HPP
#define AIE_HPP

#include "boost/hana.hpp"

#include "detail/common.hpp"
#include "detail/hardware.hpp"
#include "detail/service.hpp"
#include <array>
#include <vector>

/// The top-level file of the runtime. aie.hpp contains of the API a user should
/// use. aie++ can be compiled with many different implementations this
/// file(aie.hpp) contains only the common parts of the runtime. The internal
/// API implemented by each back-end is defined in common.hpp.

/// Types in *_impl_fallback are example of the API that need to be implemented
/// by a backend. It should be appear in aie.hpp
/// Types in *_impl are implementation by a backend of an API.

/// here we select which include to use based on how aie++ is compiled
#ifdef __AIE_FALLBACK__
// fallback, test that the fallback API can compile
#include "detail/fallback.hpp"
#elif defined(__AIE_EMULATION__)
// emulation
#include "detail/emulation.hpp"
#elif __SYCL_DEVICE_ONLY__
// device-side on hardware
#include "detail/device_only.hpp"
#else
// host-side on hardware
#include "detail/host_only.hpp"
#endif

namespace aie {

namespace detail {

template <typename ElemTy, int X, int Y> struct placed_elem {
  static constexpr position pos { X, Y };
  ElemTy elem;
};

template <typename T> struct selected_type {
  using type = T;
};

template <typename T>
concept is_selected_type = is_instance_of_type<T, selected_type>::value;

template <typename T>
concept is_type_selector = requires(T s) {
  { s.template operator()<0, 0>() } -> is_selected_type;
};

/// Type containing all the constexpr and type information needed by tile types.
template <typename TypeSelectorTy, int size_X, int size_Y, typename ServiceTy>
struct tile_type_info {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;

  /// Implementation detail used to figure out the return type of the lambda
  /// used in the aie::queue::submit
  template <int x, int y, typename T> static auto get_type(T l) {
    return l.template operator()<x, y>();
  }

  template <typename T> struct type_selector {
    using type = typename T::type;
  };

  /// enforce that void should be treated as detail::out_of_bounds
  template <> struct type_selector<void> {
    using type = detail::out_of_bounds;
  };

  /// The storage type used by the tile at X, Y if X, Y is in range
  template <int X, int Y>
  using TileDataTy = typename type_selector<decltype(get_type<X, Y>(
      std::declval<TypeSelectorTy>()))>::type;

  /// Type of the memory tile at position X, Y
  template <int X, int Y>
  using tile_data =
      std::conditional_t <
       X >= 0 && X < size_X &&
       Y >= 0 && Y < size_Y, TileDataTy<X, Y>, detail::out_of_bounds>;

  using service_type = ServiceTy;
};

/// Storage for all tiles and memory tiles.
template <typename TypeSelectorTy,
          template <typename, int X, int Y> typename HostTileTy, int size_X,
          int size_Y, typename ServiceTy>
struct layout_storage {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;

  /// tuple of all valid tile coordinates.
  static constexpr auto tile_coordinates = boost::hana::cartesian_product(
      boost::hana::make_tuple(boost::hana::range_c<int, 0, sizeX>,
                              boost::hana::range_c<int, 0, sizeY>));
  using type_info = tile_type_info<TypeSelectorTy, sizeX, sizeY, ServiceTy>;

  /// Implementation detail to generate a tuple of all tile storage types
  static auto generate_storage() {
    return boost::hana::transform(tile_coordinates, [&](auto coord) {
      return placed_elem<
          typename type_info::template TileDataTy<boost::hana::at_c<0>(coord),
                                                  boost::hana::at_c<1>(coord)>,
          boost::hana::at_c<0>(coord), boost::hana::at_c<1>(coord)> {};
    });
  }
  /// a tuple of all tile storage types
  decltype(generate_storage()) tile_storage = generate_storage();

  detail::out_of_bounds out_of_bound_storage;

  template <int LinearId> auto& storage_at() {
    return boost::hana::at_c<LinearId>(tile_storage);
  }

  static auto constexpr linear_id(int x, int y) { return x + sizeX * y; }

  template <int X, int Y> auto& storage_at() {
    return storage_at<linear_id(X, Y)>();
  }

  /// get the storage of X, Y
  template <int X, int Y> auto& get_storage() {
    if constexpr (X < 0 || Y < 0 || X >= sizeX || Y >= sizeY)
      return out_of_bound_storage;
    return storage_at<X, Y>().elem;
  }

  static auto generate_tiles() {
    return boost::hana::transform(tile_coordinates, [&](auto coord) {
      return placed_elem<HostTileTy<type_info, boost::hana::at_c<0>(coord),
                                     boost::hana::at_c<1>(coord)>,
                          boost::hana::at_c<0>(coord),
                          boost::hana::at_c<1>(coord)> {};
    });
  }
  decltype(generate_tiles()) tiles = generate_tiles();

  void init(detail::device_impl& device) {
    /// inform the device_impl of all the tile memories
    boost::hana::for_each(
        tile_storage, [&](auto& ts) { device.add_storage(ts.pos, &ts.elem); });

    /// init all the host_tiles
    boost::hana::for_each(tiles, [&](auto& ts) { ts.elem.init(device); });
  }
  layout_storage(detail::device_impl& device) { init(device); }
};

template <typename... ServicesTy> struct service_storage {
  using list_t = detail::type_seq<ServicesTy...>;
  using service_list_t = list_t::template to<detail::service_list_info>;
  std::tuple<ServicesTy...> data;
  template <typename... Ts, typename... T2s>
  service_storage<Ts..., ServicesTy..., T2s...> prepend(T2s... t2) {
    return { std::apply(
        [&](auto... elems) {
          return std::make_tuple(Ts {}..., std::move(t2)...,
                                 std::move(elems)...);
        },
        std::move(data)) };
  }
};

template <typename T>
concept is_service_storage = is_instance_of_type<T, service_storage>::value;

} // namespace detail

/// The adaptor turns a handle_impl implementing only the bare minimum into a
/// user-facing handle with a full pretty API.
using device_mem_handle =
    detail::device_mem_handle_adaptor<detail::device_mem_handle_impl>;

/// similar to std::lock_guard but it is adapted to use aie::*_lock_impl instead
/// of std::*mutex
/// TODO: the lock_guard* needs to be refactored into read_lock_guard and write_lock_guard.
template <typename LockTy> struct lock_guard_ex : detail::no_move {
 private:
  LockTy impl;

 public:
  template <typename> friend struct unlock_guard_ex;
  lock_guard_ex(LockTy l)
      : impl(l) {
    impl.acquire();
  }
  ~lock_guard_ex() { impl.release(); }
};

template <typename LockTy> lock_guard_ex(LockTy) -> lock_guard_ex<LockTy>;

namespace detail {

template <typename> struct get_inner {};

template <typename InnerT, template <typename> typename OuterT>
struct get_inner<OuterT<InnerT>> {
  using type = InnerT;
};

template <typename TypeInfoTy, int X, int Y> struct tile_base {
  static constexpr int x() { return X; }
  static constexpr int y() { return Y; }
  static constexpr position pos() { return { X, Y }; }

  static constexpr int size_x() { return TypeInfoTy::sizeX; }
  static constexpr int size_y() { return TypeInfoTy::sizeY; }
  /// Check id the neighbor is direction d is valid to access
  static constexpr bool has_mem(dir d) {
    return pos().on(d).is_valid(size_x(), size_y());
  }
  static constexpr bool has_neighbor(dir d) {
    return (pos() + get_simple_offset(d)).is_valid(size_x(), size_y());
  }
};

} // namespace detail

template <typename T> struct add_to_api_base {
 protected:
  /// get the device tile for a service's add_to_api mix-in
  auto& tile() {
    return *static_cast<typename detail::get_inner<T>::type*>(
                static_cast<T*>(this))
                ->dt();
  }
};

namespace detail {
enum stream_operation {
  reading,
  writing,
};
}

/// Type used to manipulate a tile on the device from the device itself.
/// TypeInfoTy contains all type and constexpr information used by the
/// device_tile.
template <typename TypeInfoTy, int X, int Y>
struct device_tile
    : private detail::device_tile_impl
    , public detail::tile_base<TypeInfoTy, X, Y> {
 private:
  using impl = detail::device_tile_impl;
  using base = detail::tile_base<TypeInfoTy, X, Y>;

  /// Write or Read data of any size using fixed-size stream operations
  template <unsigned FixedSize, detail::stream_operation op_kind,
            typename OpFunc>
  static void
  stream_operation(typename std::conditional<op_kind == detail::reading, char,
                                             const char>::type* ptr,
                   unsigned size, OpFunc stream_operation) {
    unsigned idx = 0;

    /// While we can make full size stream transfer, do it.
    for (; idx + FixedSize <= size; idx += FixedSize)
      stream_operation(ptr + idx);

    /// If we have leftover bytes to send
    if (idx < size) {
      /// the stream operations are fixed size so we need a buffer of sufficient
      /// size.
      std::array<char, FixedSize> buffer {};

      /// When writing to a stream, copy the remaining data into the buffer.
      /// then send the whole buffer on the stream
      if constexpr (op_kind == detail::writing)
        std::memcpy(buffer.data(), ptr + idx, size - idx);

      stream_operation(buffer.data());

      /// When reading from a stream, receive the data into the buffer, then
      /// copy the relevant part of the buffer to the right place.
      if constexpr (op_kind == detail::reading)
        std::memcpy(ptr + idx, buffer.data(), size - idx);
    }
  }

  /// Core logic of barriers. propagate a token from start_end to 
  template<dir start_end>
  void barrier_impl(int lock_id) {
    static constexpr dir opposite = opposite_dir(start_end);
    // Propagate a token from South to North and back
    // All tile except the bottom one wait. 0, 0 stuck on north, lock 0, 1 stuck on self lock
    if constexpr (has_neighbor(start_end)) {
      // Wait for the Southern neighbour to be ready
      lock(lock_id).acquire_with_value(true);
    }
    // All tile except the top one wait.
    if constexpr (has_neighbor(opposite)) {
      lock(opposite, lock_id).acquire_with_value(false);
      // Unleash the Northern neighbour
      lock(opposite, lock_id).release_with_value(true);
      // Wait for the Northern neighbour to acknowledge
      lock(opposite, lock_id).acquire_with_value(false);
    }
    // All tile except the bottom one wait.
    if constexpr (has_neighbor(start_end)) {
      // Acknowledge to the Southern neighbour
      lock(lock_id).release_with_value(false);
      // The previous release only changes the value. This release will also
      // unlock the lock
      lock(lock_id).release_with_value(false);
    }
  }

  template <dir d> auto get_tile_type() {
    constexpr auto new_pos = pos().on(d);
    return detail::selected_type<typename TypeInfoTy::template tile_data<new_pos.x, new_pos.y>>{};
  }

 public:
  using impl::init;
  using self_memory_tile = typename TypeInfoTy::template tile_data<X, Y>;

  using base::x;
  using base::y;
  using base::pos;

  /// Using the constexpr API will prevent kernel merging in many cases,
  /// So we also provide a non-constexpr way to get the x and y
  int dyn_x() { return impl::x_coord(); }
  int dyn_y() { return impl::y_coord(); }
  position dyn_pos() { return { dyn_x(), dyn_y() }; }

  using base::size_x;
  using base::size_y;
  using base::has_mem;
  using base::has_neighbor;

  /// access the neighbor's memory tile
  template <dir d> auto& get_mem() {
    using RetTy = decltype(get_tile_type<d>())::type;
    return *(RetTy*)impl::get_mem_addr<d>();
  }
  auto& mem() { return get_mem<dir::self>(); }
  auto& mem_north() { return get_mem<dir::north>(); }
  auto& mem_south() { return get_mem<dir::south>(); }
  auto& mem_east() { return get_mem<dir::east>(); }
  auto& mem_west() { return get_mem<dir::west>(); }
  auto& mem_side() {
    if (pos().get_parity() == parity::east)
      return get_mem<dir::east>();
    else
      return get_mem<dir::east>();
  }

  /// Write an object of type ElemTy to stream
  template <typename ElemTy>
  void stream_write(const ElemTy& value, int stream_id) {
    static_assert(std::is_trivially_copyable_v<ElemTy>,
                  "ElemTy cannot passed by streams");
    /// The hardware supports writing to a stream by blocks of 16 bytes.
    stream_operation<16, detail::writing>(
        reinterpret_cast<const char*>(std::addressof(value)), sizeof(ElemTy),
        [this, stream_id](const char* ptr) {
          impl::stream_write16(ptr, stream_id);
        });
  }
  /// Read an object of type ElemTy from stream
  template <typename ElemTy> ElemTy stream_read(int stream_id) {
    static_assert(std::is_trivially_copyable_v<ElemTy>,
                  "ElemTy cannot passed by streams");
    ElemTy value;
    /// The hardware supports reading to a stream by blocks of 16 bytes.
    stream_operation<16, detail::reading>(
        reinterpret_cast<char*>(std::addressof(value)), sizeof(ElemTy),
        [this, stream_id](char* ptr) { impl::stream_read16(ptr, stream_id); });
    return value;
  }
  /// Write an object of type ElemTy to the cascade stream
  template <typename ElemTy> void cascade_write(const ElemTy& value) {
    // static_assert(!is_cascade_end(), "There is nothing to write for.");
    static_assert(std::is_trivially_copyable_v<ElemTy>,
                  "ElemTy cannot passed by streams");
    /// The hardware supports writing to the cascade stream by blocks of 48
    /// bytes.
    stream_operation<48, detail::writing>(
        reinterpret_cast<const char*>(std::addressof(value)), sizeof(ElemTy),
        [this](const char* ptr) { impl::cascade_write48(ptr); });
  }

  /// Read an object of type ElemTy from the cascade stream
  template <typename ElemTy> ElemTy cascade_read() {
    // static_assert(!is_cascade_start(), "There is nothing to read from");
    static_assert(std::is_trivially_copyable_v<ElemTy>,
                  "ElemTy cannot passed by streams");
    ElemTy value;
    /// The hardware supports reading to the cascade stream by blocks of 48
    /// bytes.
    stream_operation<48, detail::reading>(
        reinterpret_cast<char*>(std::addressof(value)), sizeof(ElemTy),
        [this](char* ptr) { impl::cascade_read48(ptr); });
    return value;
  }
  template <typename ElemTy> void cascade_read_into(ElemTy& out) {
    out = cascade_read<ElemTy>();
  }

  detail::device_lock_impl lock(int i) { return lock(dir::self, i); }
  detail::device_lock_impl lock(dir d, int i) {
    return impl::lock(pos(), d, i);
  }

  void vertical_barrier(int lock_id = 15) {
    barrier_impl<dir::south>(lock_id);
    /// barrier_impl<dir::north>(lock_id) would also be correct.
  }

  void horizontal_barrier(int lock_id = 14) {
    if constexpr (y() & 1)
      barrier_impl<dir::west>(lock_id);
    else
      barrier_impl<dir::east>(lock_id);
  }

  void full_barrier(int id0 = 14, int id1 = 15) {
    vertical_barrier(id0);
    horizontal_barrier(id1);
  }

  /// Execute the service that take a type T as data in its act_on_data function
  /// If chained is true the host will wait until for an other request from the
  /// same device. This is used to prevent interleaving during a sequence of
  /// requests.
  template <typename T> auto perform_service(T data, bool chained = false) {
    T local = data;
    /// lookup what type of service sends this data;
    using service_t = TypeInfoTy::service_type::template get_info_t<
        TypeInfoTy::service_type::data_seq::template get_index<
            std::decay_t<T>>>::service_t;
    /// Call on to the implementation to execute the service correct
    return impl::perform_service<service_t, typename TypeInfoTy::service_type>(
        local, chained);
  }
  TypeInfoTy::service_type::template service_list_accessor<
      device_tile, detail::multi_service_accessor>
  service() {
    return { this };
  }
};

/// Type used to manipulate a tile on the device from the host
/// TypeInfoTy contains all type and constexpr information used by the
/// host_tile.
template <typename TypeInfoTy, int X, int Y>
struct host_tile : public detail::tile_base<TypeInfoTy, X, Y> {
 private:
  device_tile<TypeInfoTy, X, Y> dt;
  detail::host_tile_impl impl;
  detail::device_impl* dev_impl;

 public:
  static constexpr position pos() { return { X, Y }; }
  void init(detail::device_impl& global) {
    dev_impl = &global;
    dt.init(global, pos());
    impl.init(global, pos());
  }

  /// access the memory tile on the host and notify that it needs to be sent to
  /// the device.
  auto& mem() {
    using RetTy = typename decltype(dt)::self_memory_tile;
    void* ptr = dev_impl->get_mem(pos());
    impl.notify_has_accessed_mem(ptr, sizeof(RetTy));
    return *reinterpret_cast<RetTy*>(ptr);
  }

  detail::host_lock_impl lock(int i) { return impl.lock(i); }

  /// execute a kernel on the device
  template <typename F> void single_task(F&& func) { impl.execute(func, dt); }
  /// Used as implementation detail. this should be refactored such that it
  /// becomes private
  void register_accessor(const detail::accessor_common& acc) {
    impl.register_accessor(acc);
  }
};

template <typename T> detail::selected_type<T> tile_storage() { return {}; }

template <typename... Ts, typename... T2s>
detail::service_storage<Ts..., T2s...> add_service(T2s... t2) {
  detail::service_storage<> tmp;
  return tmp.prepend<Ts...>(std::move(t2)...);
}

template <int size_X, int size_Y> struct device {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;
};

template <typename DevTy> struct queue {
  DevTy dev;
  queue(DevTy d)
      : dev { d } {}

  /// Submit that uses the same struct for all memory tiles
  template <typename TileDataTy = aie::detail::out_of_bounds, typename F = void,
            detail::is_service_storage ServiceListT = detail::service_storage<>>
  void submit_uniform(F&& func,
                      ServiceListT services = detail::service_storage<> {}) {
    submit([]<int X, int Y>() { return tile_storage<TileDataTy>(); },
           std::forward<F>(func), services);
  }

  /// Submit that uses difference instances of the same template for all memory
  /// tiles
  template <template <int, int> typename TileDataTy, typename F,
            detail::is_service_storage ServiceListT = detail::service_storage<>>
  void submit_hetero(F&& func,
                     ServiceListT services = detail::service_storage<> {}) {
    submit([]<int X, int Y>() { return tile_storage<TileDataTy<X, Y>>(); },
           std::forward<F>(func), services);
  }

  /// Submit without any access to memory tiles;
  template <typename F,
            detail::is_service_storage ServiceListT = detail::service_storage<>>
  void submit(F&& func, ServiceListT services = detail::service_storage<> {}) {
    submit([]<int X, int Y>() { return tile_storage<detail::out_of_bounds>(); },
           std::forward<F>(func), services);
  }

  /// Fully generic submit using a lambda to describe which type to use for
  /// which memory tile
  template <detail::is_type_selector TypeSelectorTy, typename F,
            typename ServiceStorageT = detail::service_storage<>>
  void submit(TypeSelectorTy&& type_selector, F&& func,
              ServiceStorageT services_tmp = detail::service_storage<> {}) {
    /// The device back-end depends on done_service and send_log_service being
    /// the first 2 service_list in the list.
    auto services =
        services_tmp
            .template prepend<detail::done_service<device_mem_handle>,
                              detail::send_log_service<device_mem_handle>>();
    using service_impl = decltype(services)::service_list_t;

    /// Access the device physical or emulated
    detail::device_impl impl(DevTy::sizeX, DevTy::sizeY);

    /// Create all the storage
    detail::layout_storage<TypeSelectorTy, host_tile, DevTy::sizeX,
                           DevTy::sizeY, service_impl>
        storage(impl);

    /// Execute the func for all host tiles
    boost::hana::for_each(storage.tiles, [&](auto& ts) { func(ts.elem); });
    impl.wait_all(std::move(services));
  }
};

template <typename DevTy> queue(DevTy&) -> queue<DevTy>;

/// For now buffers are simple vector. this is temporary
template <typename T> using buffer = std::vector<T>;

enum access_mode {
  read_only = 1 << 0,
  write_only = 1 << 1,
  read_write = read_only | write_only,
};

template <typename T> struct accessor;

template <typename T, typename HostTileTy> struct buffer_range {
 private:
  friend struct accessor<T>;
  HostTileTy& tile;
  const buffer<T>& buff;
  uint32_t start_read;
  uint32_t end_read;
  uint32_t start_write;
  uint32_t end_write;

  uint32_t all_range_start() { return std::min(start_read, start_write); }

  uint32_t all_range_end() { return std::max(end_read, end_write); }

  uint32_t all_range_size() { return all_range_end() - all_range_start(); }

  auto* data() { return buff.data() + all_range_start(); }

  uint32_t get_write_start() { return start_write - all_range_start(); }

  uint32_t get_write_end() { return end_write - all_range_start(); }

  uint32_t get_write_size() { return get_write_end() - get_write_start(); }

 public:
  buffer_range(HostTileTy& t, const buffer<T>& b)
      : tile(t)
      , buff(b)
      , start_read(0)
      , end_read(buff.size())
      , start_write(0)
      , end_write(buff.size()) {}

  /// Specify that only the part of the buffer from start to end will read by the kernel.
  /// The runtime will only send the read part of the buffer to the device.
  buffer_range read_range(int start, int end) {
    start_read = start;
    end_read = end;
    return *this;
  }

  /// Specify the range start to end inside the read range that will also be
  /// written to. The runtime will write back the part that is written to back
  /// to the host.
  buffer_range write_range(int start, int end) {
    start_write = start;
    end_write = end;
    return *this;
  }
};

template <typename T, typename HostTileTy>
buffer_range(HostTileTy& t, const buffer<T>& b) -> buffer_range<T, HostTileTy>;

/// The layout of the accessor must be the same on device and host. Because the
/// lambda must be the same on both side, so the size and alignment must match
template <typename T>
struct alignas(8) __SYCL_TYPE(aie_accessor) accessor
    : detail::accessor_common {
 private:
  using base = detail::accessor_common;

 public:
  template <typename HostTileTy>
  accessor(buffer_range<T, HostTileTy> range)
      : base { range.all_range_size(), (uint32_t)sizeof(T), (char*)range.data(),
               range.get_write_start(), range.get_write_size() } {}
  template <typename HostTileTy>
  accessor(HostTileTy& tile, const buffer<T>& buff, access_mode am = read_write)
      : base { (uint32_t)buff.size(), (uint32_t)sizeof(T),
               (char*)buff.data() } {
    static_assert(std::is_trivially_copyable_v<T>,
                  "cannot safely be copied to the device");
    /// calling register_accessor is not needed in device execution because we
    /// have compiler support for introspecting the layout of the lambda.
    tile.register_accessor(*this);
  }
  /// Return the number of elements in the bounding box of elements declared as
  /// read and written
  unsigned size() const { return base::size(); }
  /// accessor one element of the bounding box of elements declared as read and
  /// written
  T& operator[](unsigned idx) const { return get_data()[idx]; }
  /// get a pointer to the start of the bounding box of elements declared as
  /// read and written
  T* get_data() const { return (T*)base::get_ptr(); }
};

namespace detail {
/// validate that the accessor has the correct layout
detail::assert_equal<sizeof(accessor<int>), 8> check_sizeof_accessor;
detail::assert_equal<alignof(accessor<int>), 8> check_alignof_accessor;
} // namespace detail

} // namespace aie

#endif
