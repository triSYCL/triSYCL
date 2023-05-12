
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
/// by a backend. it should be appear in aie.hpp
/// Types in *_impl are implementation by a backend of an API.

/// here we select which include to use based on how aie++ is compiled
#ifdef __AIE_FALLBACK__
// fallback, test that the fallback API can compile
#include "detail/fallback.hpp"
#elif defined(__ACAP_EMULATION__)
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

using device_mem_handle = detail::device_mem_handle_adaptor<detail::device_mem_handle_impl>;

/// similar to std::lock_guard but it is adapted to use aie::*_lock_impl instead
/// of std::*mutex
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

/// Mix-in to provide the APIs of all services easily to the user
template<typename DT, typename ... Ts>
struct multi_service_accessor : Ts::template add_to_dev_handle<multi_service_accessor<DT, Ts...>>... {
  private:
  DT* device_tile;
  public:
  multi_service_accessor(DT* d) : device_tile(d) {}
  DT* dt() { return device_tile; }
};

/// Type used to manipulate a tile on the device from the device itself.
/// TypeInfoTy contains all type and constexpr information used by the
/// device_tile.
template <typename TypeInfoTy, int X, int Y>
struct device_tile : private detail::device_tile_impl {
 private:
  using impl = detail::device_tile_impl;

  /// Write or Read data of any size using fixed-size stream operations
  template <unsigned FixedSize, bool is_read, typename OpFunc>
  static void stream_operation(
      typename std::conditional<is_read, char, const char>::type* ptr,
      unsigned size, OpFunc stream_operation) {
    unsigned idx = 0;

    /// While we can make full size stream transfer do it.
    for (; idx + FixedSize <= size; idx += FixedSize)
      stream_operation(ptr + idx);

    /// If we have leftover bytes to send
    if (idx < size) {
      /// the stream operations are fixed size so we need a buffer of sufficient
      /// size.
      std::array<char, FixedSize> buffer {};

      /// When writing to a stream, copy the remaining data into the buffer.
      /// then send the whole buffer on the stream
      if constexpr (!is_read)
        std::memcpy(buffer.data(), ptr + idx, size - idx);

      stream_operation(buffer.data());

      /// When reading from a stream, receive the data into the buffer, then
      /// copy the relevant part of the buffer in the right place.
      if constexpr (is_read)
        std::memcpy(ptr + idx, buffer.data(), size - idx);
    }
  }

 public:
  using impl::init;
  using self_memory_tile = typename TypeInfoTy::template tile_data<X, Y>;
  template <hw::dir d> auto get_tile_type() {
    constexpr auto new_pos = hw::position { X, Y }.moved(d);
    return typename TypeInfoTy::template tile_data<new_pos.x, new_pos.y> {};
  }

  static constexpr int x() { return X; }
  static constexpr int y() { return Y; }
  static constexpr hw::position get_pos() { return { X, Y }; }

  /// Using the constexpr API will prevent kernel merging in many cases,
  /// So we also provide a non-constexpr way to get the x and y
  int dyn_x() { return impl::x_coord(); }
  int dyn_y() { return impl::y_coord(); }
  hw::position get_dyn_pos() { return { dyn_x(), dyn_y() }; }

  /// layout of the aie's in use
  static constexpr int size_x() { return TypeInfoTy::sizeX; }
  static constexpr int size_y() { return TypeInfoTy::sizeY; }
  /// Check id the neighbor is direction d is valid to access
  static constexpr bool has_mem(hw::dir d) {
    return get_pos().moved(d).is_valid(size_x(), size_y());
  }

  /// access the neighbor's memory tile
  template <typename hw::dir d> auto& get_mem() {
    using RetTy = decltype(get_tile_type<d>());
    return *(RetTy*)impl::get_mem_addr<d>();
  }
  auto& mem() { return get_mem<hw::dir::self>(); }
  auto& mem_north() { return get_mem<hw::dir::north>(); }
  auto& mem_south() { return get_mem<hw::dir::south>(); }
  auto& mem_east() { return get_mem<hw::dir::east>(); }
  auto& mem_west() { return get_mem<hw::dir::west>(); }
  auto& mem_side() {
    if (get_pos().get_parity() == hw::parity::east)
      return get_mem<hw::dir::east>();
    else
      return get_mem<hw::dir::east>();
  }

  /// Write an object of type ElemTy to stream
  template <typename ElemTy>
  void stream_write(const ElemTy& value, int stream_id) {
    static_assert(std::is_trivially_copyable_v<ElemTy>,
                  "ElemTy cannot passed by streams");
    /// The hardware supports writing to a stream by blocks of 16 bytes.
    stream_operation<16, /*is_read*/ false>(
        reinterpret_cast<const char*>(std::addressof(value)),
        sizeof(ElemTy), [this, stream_id](const char* ptr) {
          impl::stream_write16(ptr, stream_id);
        });
  }
  /// Read an object of type ElemTy from stream
  template <typename ElemTy> ElemTy stream_read(int stream_id) {
    static_assert(std::is_trivially_copyable_v<ElemTy>,
                  "ElemTy cannot passed by streams");
    ElemTy value;
    /// The hardware supports reading to a stream by blocks of 16 bytes.
    stream_operation<16, /*is_read*/ true>(
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
    stream_operation<48, /*is_read*/ false>(
        reinterpret_cast<const char*>(std::addressof(value)),
        sizeof(ElemTy),
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
    stream_operation<48, /*is_read*/ true>(
        reinterpret_cast<char*>(std::addressof(value)), sizeof(ElemTy),
        [this](char* ptr) { impl::cascade_read48(ptr); });
    return value;
  }

  detail::device_lock_impl get_lock(int i) {
    return get_lock(hw::dir::self, i);
  }
  detail::device_lock_impl get_lock(hw::dir d, int i) {
    return impl::get_lock(d, i);
  }
  // void vertical_barrier(int lock = 15) {
  //   // Propagate a token from South to North and back
  //   // All tile except the bottom one wait.
  //   aie::detail::log("vertical_barrier: start\n");
  //   if constexpr (has_neighbor(hw::dir::south)) {
  //     aie::detail::log("vertical_barrier: if 0 0\n");
  //     // Wait for the Southern neighbour to be ready
  //     get_lock(lock).acquire_with_value(true);
  //     aie::detail::log("vertical_barrier: if 0 1\n");
  //   }
  //   // All tile except the top one wait.
  //   if constexpr (has_neighbor(hw::dir::north)) {
  //     aie::detail::log("vertical_barrier: if 1 0\n");
  //     get_lock(hw::dir::north, lock).acquire_with_value(false);
  //     // Unleash the Northern neighbour
  //     aie::detail::log("vertical_barrier: if 1 1\n");
  //     get_lock(hw::dir::north, lock).release_with_value(true);
  //     // Wait for the Northern neighbour to acknowledge
  //     aie::detail::log("vertical_barrier: if 1 2\n");
  //     get_lock(hw::dir::north, lock).acquire_with_value(false);
  //     aie::detail::log("vertical_barrier: if 1 3\n");
  //   }
  //   // All tile except the bottom one wait.
  //   if constexpr (has_neighbor(hw::dir::south)) {
  //     aie::detail::log("vertical_barrier: if 2 0\n");
  //     // Acknowledge to the Southern neighbour
  //     get_lock(lock).release_with_value(false);
  //     aie::detail::log("vertical_barrier: if 2 1\n");
  //   }
  //   aie::detail::log("vertical_barrier: final 1\n");
  //   /// Reset the lock for the next barrier.
  //   // get_lock(lock).release_with_value(false);
  //   // aie::detail::log("vertical_barrier: final 2\n");
  // }

  // template <typename T, typename... Ts> auto perform_service(Ts&&... ts) {
  //   using info_t = detail::service_info<T>;
  //   return impl::perform_service<T>(
  //       typename info_t::data_t { std::forward<Ts>(ts)... });
  // }

  template <typename T> auto perform_service(T data) {
    T local = data;
    /// lookup what type of service sends this data;
    using service_t = TypeInfoTy::service_type::template get_info_t<
        TypeInfoTy::service_type::data_seq::template get_index<std::decay_t<T>>>::
        service_t;
    return impl::perform_service<service_t, typename TypeInfoTy::service_type>(local);
  }
  TypeInfoTy::service_type::template service_list_accessor<device_tile, multi_service_accessor> service() {
    return { this };
  }
};

/// Type used to manipulate a tile on the device from the host
/// TypeInfoTy contains all type and constexpr information used by the
/// host_tile.
template <typename TypeInfoTy, int X, int Y> struct host_tile {
 private:
  device_tile<TypeInfoTy, X, Y> dt;
  detail::host_tile_impl impl;
  detail::device_impl* dev_impl;

 public:
  static constexpr hw::position get_pos() { return { X, Y }; }
  void init(detail::device_impl& global) {
    dev_impl = &global;
    dt.init(global, get_pos());
    impl.init(global, get_pos());
  }

  /// access the memory tile on the host and notify that it needs to be send to
  /// the device.
  auto& mem() {
    using RetTy = typename decltype(dt)::self_memory_tile;
    void* ptr = dev_impl->get_mem(get_pos());
    impl.notify_has_accessed_mem(ptr, sizeof(RetTy));
    return *reinterpret_cast<RetTy*>(ptr);
  }

  detail::host_lock_impl get_lock(int i) { return impl.get_lock(i); }

  /// execute a kernel on the device
  template <typename F> void single_task(F&& func) { impl.execute(func, dt); }
  /// Used as implementation detail. this should be refactor such that it
  /// becomes private
  void register_accessor(const detail::accessor_common& acc) {
    impl.register_accessor(acc);
  }
};

namespace detail {

template <typename ElemTy, int X, int Y> struct indexed_elem {
  static constexpr hw::position pos { X, Y };
  ElemTy elem;
};

/// Type containing all the constexpr and type information needed by tile types.
template <typename TypeSelectorTy, int size_X, int size_Y, typename ServiceTy>
struct tile_type_info {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;

  /// Implementation detail to figure out the return type of the lambda
  template <int x, int y, typename T> static auto get_type(T l) {
    return l.template operator()<x, y>();
  }

  template <typename T> struct type_selector {
    using type = typename T::type;
  };

  template <> struct type_selector<void> {
    using type = detail::out_of_bounds;
  };

  template <int X, int Y>
  using TileDataTy = typename type_selector<decltype(get_type<X, Y>(
      std::declval<TypeSelectorTy>()))>::type;

  /// Type of the memory tile at position X, Y
  template <int X, int Y>
  using tile_data =
      std::conditional_t < X >= 0 && Y >= 0 &&
      X < size_X&& Y<size_Y, TileDataTy<X, Y>, detail::out_of_bounds>;
  
  using service_type = ServiceTy;
};

/// Storage for all tiles and memory tiles.
/// Handle all the boost::hana stuff
template <typename TypeSelectorTy,
          template <typename, int X, int Y> typename HostTileTy, int size_X,
          int size_Y, typename ServiceTy>
struct layout_storage {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;
  static constexpr auto tile_coordinates = boost::hana::cartesian_product(
      boost::hana::make_tuple(boost::hana::range_c<int, 0, sizeX>,
                              boost::hana::range_c<int, 0, sizeY>));
  using type_info = tile_type_info<TypeSelectorTy, sizeX, sizeY, ServiceTy>;

  static auto generate_storage() {
    return boost::hana::transform(tile_coordinates, [&](auto coord) {
      return indexed_elem<
          typename type_info::template TileDataTy<boost::hana::at_c<0>(coord),
                                                  boost::hana::at_c<1>(coord)>,
          boost::hana::at_c<0>(coord), boost::hana::at_c<1>(coord)> {};
    });
  }
  decltype(generate_storage()) tile_storage = generate_storage();

  template <int LinearId> auto& storage_at() {
    return boost::hana::at_c<LinearId>(tile_storage);
  }

  static auto constexpr linear_id(int x, int y) { return x + sizeX * y; }

  template <int X, int Y> auto& storage_at() {
    return storage_at<linear_id(X, Y)>();
  }

  template <int X, int Y> auto get_storage_type() {
    if constexpr (X < 0 || Y < 0 || X >= sizeX || Y >= sizeY)
      return detail::out_of_bounds {};
    return storage_at<X, Y>();
  }

  static auto generate_tiles() {
    return boost::hana::transform(tile_coordinates, [&](auto coord) {
      return indexed_elem<HostTileTy<type_info, boost::hana::at_c<0>(coord),
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

template <typename T> struct selected_type {
  using type = T;
};

template <typename...Ts> struct type_list {
};

} // namespace detail

template <typename T> detail::selected_type<T> select() { return {}; }

template <typename... Ts> detail::type_list<Ts...> add_service() { return {}; }

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
            typename ServiceListT = detail::type_list<>>
  void submit_uniform(F&& func,
                      ServiceListT services = detail::type_list<> {}) {
    submit([]<int X, int Y>() { return select<TileDataTy>(); },
           std::forward<F>(func), services);
  }
  /// Submit that uses difference instances of the same template for all memory
  /// tiles
  template <template <int, int> typename TileDataTy, typename F,
            typename ServiceListT = detail::type_list<>>
  void submit_hetero(F&& func, ServiceListT services = detail::type_list<> {}) {
    submit([]<int X, int Y>() { return select<TileDataTy<X, Y>>(); },
           std::forward<F>(func), services);
  }

  /// Submit without any access to memory tiles;
  template <typename F, typename ServiceListT = detail::type_list<>>
  void submit(F&& func, ServiceListT services = detail::type_list<> {}) {
    submit([]<int X, int Y>() { return select<detail::out_of_bounds>(); },
           std::forward<F>(func), services);
  }

  /// fully generic submit using a lambda to describe which type to use for
  /// which memory tile
  template <typename TypeSelectorTy, typename F, typename ... ExtraServiceTys>
  void submit(TypeSelectorTy&& type_selector, F&& func, detail::type_list<ExtraServiceTys...> = detail::type_list<>{}) {
    /// The device back-end depends on done_service and send_log_service being the first
    /// 2 service_list in the list.
    using service_impl =
        detail::service_list_info<detail::done_service<device_mem_handle>,
                                  detail::send_log_service<device_mem_handle>,
                                  ExtraServiceTys...>;
    /// access the device physical or emulated
    detail::device_impl impl(DevTy::sizeX, DevTy::sizeY);

    /// create all the storage
    detail::layout_storage<TypeSelectorTy, host_tile, DevTy::sizeX,
                           DevTy::sizeY, service_impl>
        storage(impl);

    /// execute the func for all host tiles
    boost::hana::for_each(storage.tiles, [&](auto& ts) { func(ts.elem); });
    impl.wait_all<service_impl>();
  }
};

template <typename DevTy> queue(DevTy&) -> queue<DevTy>;

/// for now buffers are simple vector. this is temporary
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
  public:
   buffer_range(HostTileTy& t, const buffer<T>& b)
       : tile(t)
       , buff(b)
       , start_read(0)
       , end_read(buff.size())
       , start_write(0)
       , end_write(0) {}
  buffer_range read_range(int start, int end) {
    start_read = start;
    end_read = end;
    return *this;
  }
  buffer_range write_range(int start, int end) {
    start_write = start;
    end_write = end;
    return *this;
  }
};

template <typename T, typename HostTileTy>
buffer_range(HostTileTy& t, const buffer<T>& b) -> buffer_range<T, HostTileTy>;

/// The layout of the accessor must be the same on device and host. because the
/// lambda must be the same on both side, so the size and alignment must match
template <typename T>
struct alignas(8) __SYCL_TYPE(acap_accessor) accessor
    : detail::accessor_common {
 private:
  using base = detail::accessor_common;

 public:
  template <typename HostTileTy>
  accessor(buffer_range<T, HostTileTy> range)
      : base { (uint32_t)range.buff.size() - range.start_read - range.end_read,
               (uint32_t)sizeof(T), (char*)range.buff.data() + range.start_read,
               range.start_write - range.start_read,
               range.end_write - range.start_read } {}
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
  unsigned get_size() const { return base::size(); }
  T& operator[](unsigned idx) { return get_data()[idx]; }
  const T& operator[](unsigned idx) const { return get_data()[idx]; }
  T* get_data() { return (T*)base::get_ptr(); }
  const T* get_data() const { return (const T*)base::get_ptr(); }
};

namespace detail {
/// validate that the accessor has the correct layout
detail::assert_equal<sizeof(accessor<int>), 8> check_sizeof_accessor;
detail::assert_equal<alignof(accessor<int>), 8> check_alignof_accessor;
} // namespace detail

} // namespace aie
