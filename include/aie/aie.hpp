
#include "boost/hana.hpp"

#include "detail/common.hpp"
#include "detail/hardware.hpp"
#include <array>

#ifdef __ACAP_EMULATION___
// emulation
#include "detail/emulation.hpp"
#else
#ifdef __SYCL_DEVICE_ONLY__
// device-side on hardware
#include "detail/device_only.hpp"
#else
// host-side on hardware
#include "detail/host_only.hpp"
#endif
#endif

namespace aie {

template <typename CRTP>
struct device_tile_common : private detail::device_tile_impl {
 private:
  using impl = detail::device_tile_impl;
  CRTP* get() { return static_cast<CRTP*>(this); }

  template <unsigned OpSize, bool is_read, typename OpFunc>
  static void stream_operation(
      typename std::conditional<is_read, char, const char>::type* ptr,
      unsigned size, OpFunc func) {
    std::array<char, OpSize> buffer;
    buffer.fill(0);
    unsigned idx = 0;
    for (; idx + OpSize <= size; idx += OpSize)
      func(ptr + idx);
    if (idx < size) {
      if constexpr (!is_read)
        std::memcpy(buffer.data(), ptr + idx, size - idx);
      func(buffer.data());
      if constexpr (is_read)
        std::memcpy(ptr + idx, buffer.data(), size - idx);
    }
  }

 protected:
  using impl::init;

 public:
  template <typename hw::dir d> auto& get_memory() {
    using RetTy = decltype(get()->template get_tile_type<d>());
    return *(RetTy*)impl::get_mem_addr<d>();
  }
  auto& mem() { return get_memory<hw::dir::self>(); }
  auto& mem_north() { return get_memory<hw::dir::north>(); }
  auto& mem_south() { return get_memory<hw::dir::south>(); }
  auto& mem_east() { return get_memory<hw::dir::east>(); }
  auto& mem_west() { return get_memory<hw::dir::west>(); }
  auto& mem_side() {
    if (get()->get_pos()->get_parity() == hw::parity::east)
      return get_memory<hw::dir::east>();
    else
      return get_memory<hw::dir::east>();
  }

  template <typename SpecifiedTy>
  void stream_write(const SpecifiedTy& value, int stream_id) {
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    stream_operation<16, /*is_read*/ false>(
        reinterpret_cast<const char*>(std::addressof(value)),
        sizeof(SpecifiedTy), [this, stream_id](const char* ptr) {
          impl::stream_write16(ptr, stream_id);
        });
  }
  template <typename SpecifiedTy> SpecifiedTy stream_read(int stream_id) {
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    SpecifiedTy value;
    stream_operation<16, /*is_read*/ true>(
        reinterpret_cast<char*>(std::addressof(value)), sizeof(SpecifiedTy),
        [this, stream_id](char* ptr) { impl::stream_read16(ptr, stream_id); });
    return value;
  }
  template <typename SpecifiedTy> void cascade_write(const SpecifiedTy& value) {
    // static_assert(!is_cascade_end(), "There is nothing to write for.");
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    stream_operation<48, /*is_read*/ false>(
        reinterpret_cast<const char*>(std::addressof(value)),
        sizeof(SpecifiedTy),
        [this](const char* ptr) { impl::cascade_write48(ptr); });
  }

  template <typename SpecifiedTy> SpecifiedTy cascade_read() {
    // static_assert(!is_cascade_start(), "There is nothing to read from");
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    SpecifiedTy value;
    stream_operation<48, /*is_read*/ true>(
        reinterpret_cast<char*>(std::addressof(value)), sizeof(SpecifiedTy),
        [this](char* ptr) { impl::cascade_read48(ptr); });
    return value;
  }
};

template <typename TileDataTy>
struct device_tile_uniform
    : public device_tile_common<device_tile_uniform<TileDataTy>> {
 private:
  using Base = device_tile_common<device_tile_uniform<TileDataTy>>;

  friend struct device_tile_common<device_tile_uniform<TileDataTy>>;
  int sizeX;
  int sizeY;

  template <hw::dir d> auto get_tile_type() { return TileDataTy {}; }

 public:
  using self_memory_tile = TileDataTy;
  static constexpr bool is_hetero() { return false; }
  void init(detail::device_impl& g, hw::position p) { Base::init(g, p); }
  int x() { return Base::impl::x_coord(); }
  int y() { return Base::impl::y_coord(); }
  hw::position get_pos() const { return { x(), y() }; }
  int size_x() { return sizeX; }
  int size_y() { return sizeX; }
  bool has_neighbor(hw::dir d) {
    return get_pos().moved(d).is_valid(size_x(), size_y());
  }
};

template <typename TypeInfoTy, int X, int Y>
struct device_tile_hetro
    : public device_tile_common<device_tile_hetro<TypeInfoTy, X, Y>> {
 private:
  friend struct device_tile_common<device_tile_hetro<TypeInfoTy, X, Y>>;
  using Base = device_tile_common<device_tile_hetro<TypeInfoTy, X, Y>>;
  template <hw::dir d> auto get_tile_type() {
    constexpr auto new_pos = hw::position { X, Y }.moved(d);
    return typename TypeInfoTy::template tile_data<new_pos.x, new_pos.y> {};
  }

 public:
  using self_memory_tile = typename TypeInfoTy::template tile_data<X, Y>;
  using Base::init;
  static constexpr bool is_hetero() { return true; }
  static constexpr int x() { return X; }
  static constexpr int y() { return Y; }
  static constexpr hw::position get_pos() { return { X, Y }; }
  static constexpr int size_x() { return TypeInfoTy::sizeX; }
  static constexpr int size_y() { return TypeInfoTy::sizeX; }
  static constexpr bool has_neighbor(hw::dir d) {
    return get_pos().moved(d).is_valid(size_x(), size_y());
  }
};

template <typename CRTP> struct host_tile_common {
 private:
  detail::host_tile_impl impl;
  CRTP* get() { return static_cast<CRTP*>(this); }

 protected:
  void init(detail::device_impl& global) {
    impl.init(global, get()->get_pos());
  }

 public:
  template <typename F> void single_task(F&& func) {
    impl.execute(func, get()->dt);
  }
  void register_accessor(const detail::accessor_common& acc) {
    impl.register_accessor(acc);
  }
};
template <typename TileDataTy>
struct host_tile_uniform
    : public host_tile_common<host_tile_uniform<TileDataTy>> {
 private:
  using Base = host_tile_common<host_tile_uniform<TileDataTy>>;
  friend struct host_tile_common<host_tile_uniform<TileDataTy>>;
  hw::position pos;
  device_tile_uniform<TileDataTy> dt;

 public:
  hw::position get_pos() const { return pos; }
  void init(detail::device_impl& global, int x, int y) {
    pos = { x, y };
    dt.init(global, pos);
    Base::init(global);
  }
};
template <typename TypeInfoTy, int X, int Y>
struct host_tile_hetero
    : public host_tile_common<host_tile_hetero<TypeInfoTy, X, Y>> {
 private:
  using Base = host_tile_common<host_tile_hetero<TypeInfoTy, X, Y>>;
  friend struct host_tile_common<host_tile_hetero<TypeInfoTy, X, Y>>;
  device_tile_hetro<TypeInfoTy, X, Y> dt;

 public:
  static constexpr hw::position get_pos() { return { X, Y }; }
  void init(detail::device_impl& global) {
    dt.init(global, get_pos());
    Base::init(global);
  }
};

template <int size_X, int size_Y> struct device {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;
  std::shared_ptr<detail::device_impl> impl;
  device()
      : impl(std::make_shared<detail::device_impl>(size_X, size_Y)) {}
};

template <typename ElemTy, int X, int Y> struct indexed_elem {
  static constexpr hw::position pos { X, Y };
  ElemTy elem;
};

template <int size_X, int size_Y> struct tile_type_info_common {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;
};

template <template <int, int> typename TileDataTy, int size_X, int size_Y>
struct tile_type_info_hetero : public tile_type_info_common<size_X, size_Y> {
  static constexpr bool is_hetro = true;
  template <int X, int Y>
  using tile_data =
      std::conditional_t < X >= 0 && Y >= 0 &&
      X < size_X&& Y<size_Y, TileDataTy<X, Y>, detail::illegal_to_access>;
};

template <typename TileDataTy, int size_X, int size_Y>
struct tile_type_info_uniform : public tile_type_info_common<size_X, size_Y> {
  static constexpr bool is_hetro = false;
  template <int X, int Y>
  using tile_data =
      std::conditional_t < X >= 0 && Y >= 0 &&
      X < size_X&& Y<size_Y, TileDataTy, detail::illegal_to_access>;
};

template <template <int, int> typename TileDataTy,
          template <typename, int X, int Y> typename HostTileTy, int size_X,
          int size_Y>
struct layout_storage {
  static constexpr int sizeX = size_X;
  static constexpr int sizeY = size_Y;
  static constexpr auto tile_coordinates = boost::hana::cartesian_product(
      boost::hana::make_tuple(boost::hana::range_c<int, 0, sizeX>,
                              boost::hana::range_c<int, 0, sizeY>));
  using type_info = tile_type_info_hetero<TileDataTy, sizeX, sizeY>;

  static auto generate_storage() {
    return boost::hana::transform(tile_coordinates, [&](auto coord) {
      return indexed_elem<
          TileDataTy<boost::hana::at_c<0>(coord), boost::hana::at_c<1>(coord)>,
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
      return detail::illegal_to_access {};
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
    boost::hana::for_each(
        tile_storage, [&](auto& ts) { device.add_storage(ts.pos, &ts.elem); });
    boost::hana::for_each(tiles, [&](auto& ts) { ts.elem.init(device); });
  }
  layout_storage(detail::device_impl& device) { init(device); }
};

template <typename DevTy> struct queue {
  DevTy dev;
  queue(DevTy d)
      : dev { d } {}
  template <typename TileDataTy = aie::detail::illegal_to_access,
            typename F = void>
  void submit_uniform(F&& func) {
    std::array<std::array<TileDataTy, DevTy::sizeY>, DevTy::sizeX> tile_storage;
    std::array<std::array<host_tile_uniform<TileDataTy>, DevTy::sizeY>,
               DevTy::sizeX>
        tiles;
    for (int x = 0; x < DevTy::sizeX; x++) {
      for (int y = 0; y < DevTy::sizeY; y++) {
        hw::position pos { x, y };
        dev.impl->add_storage(pos, &tile_storage[x][y]);
        auto& elem = tiles[x][y];
        elem.init(*dev.impl, x, y);
        func(elem);
      }
    }
    dev.impl->wait_all();
  }
  template <template <int, int> typename TileDataTy, typename F>
  void submit_hetero(F&& func) {
    layout_storage<TileDataTy, host_tile_hetero, DevTy::sizeX, DevTy::sizeY>
        storage(*dev.impl);
    boost::hana::for_each(storage.tiles, [&](auto& ts) { func(ts.elem); });
    dev.impl->wait_all();
  }
};

template <typename T>
using buffer = std::vector<T>;

enum access_mode {
  read_only = 1 << 0,
  write_only = 1 << 1,
  read_write = read_only | write_only,
};

template <typename T>
struct alignas(8) __SYCL_TYPE(acap_accessor) accessor
    : detail::accessor_common {
 private:
  using base = detail::accessor_common;
 public:
  template <typename HostTileTy>
  accessor(HostTileTy& tile, const buffer<T>& buff, access_mode am = read_write)
      : base { (unsigned)buff.size(), (unsigned)sizeof(T), (void*)buff.data() } {
    static_assert(std::is_trivially_copyable_v<T>,
                  "cannot safely be copied to the device");
    /// calling register_accessor is not needed in device execution because we
    /// have compiler support for introspecting the layout of the lambda.
    tile.register_accessor(*this);
  }
  unsigned get_size() const { return base::size; }
  T& operator[](unsigned idx) { return get_data()[idx]; }
  const T& operator[](unsigned idx) const { return get_data()[idx]; }
  T* get_data() { return (T*)base::get_ptr(); }
  const T* get_data() const { return (const T*)base::get_ptr(); }
};

namespace detail {
detail::assert_equal<sizeof(accessor<int>), 16> check_sizeof_accessor;
detail::assert_equal<alignof(accessor<int>), 8> check_alignof_accessor;
}

} // namespace aie
