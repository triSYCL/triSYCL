
#ifndef AIE_DETAIL_COMMON_HPP
#define AIE_DETAIL_COMMON_HPP

#include "hardware.hpp"
#include "meta.hpp"
#include "utils.hpp"
#include <memory>

/// Used to indicated that the feature is not or cannot be implemented
#define TRISYCL_FALLBACK                                                       \
  do {                                                                         \
    assert(false && "fallback");                                               \
    __builtin_unreachable();                                                   \
  } while (0)

/// Annotate classes that need special handling when transferred to device.
#ifdef __SYCL_DEVICE_ONLY__
#define __SYCL_TYPE(x) [[__sycl_detail__::sycl_type(x)]]
#else
#define __SYCL_TYPE(x)
#endif

namespace aie {
/// Abstraction over some pointer pointing on device data.
/// It is used for the APIs of device_mem_handle.
/// Its is intended to work on both emulation with a raw 64-bit pointer
/// and for execution on hardware with a 32-bit hw::dev_ptr<T>
template <typename T> struct generic_ptr {
  generic_ptr() = default;
#if !defined(__AIE_FALLBACK__) && !defined(__AIE_EMULATION__)
  hw::dev_ptr<T> ptr;
  generic_ptr(T*) { TRISYCL_FALLBACK; }
  generic_ptr(hw::dev_ptr<T> p)
      : ptr(p) {}
  operator generic_ptr<void>() { return hw::dev_ptr<void> { ptr }; }
#else
  T* ptr = nullptr;
  generic_ptr(T* p)
      : ptr(p) {}
  generic_ptr(hw::dev_ptr<T> p) { TRISYCL_FALLBACK; }
  operator generic_ptr<void>() const { return { (void*)ptr }; }
#endif
  operator bool() const { return (bool)ptr; }
};

/// clang warns about uses of implicitly generated deduction guides. So here are
/// some explicitly specified deduction guide that are the same as the
/// automatically generated ones.
template <typename T> generic_ptr(T*) -> generic_ptr<T>;
template <typename T> generic_ptr(hw::dev_ptr<T>) -> generic_ptr<T>;

} // namespace aie

namespace aie::detail {

/// Used as the type of a memory tile that should not be accessed.
struct out_of_bounds {};

/// handles all the meta-programming needed for the service system
/// This can be reused across all backends

///
template <typename T> struct service_info {
  using service_t = T;
  using ret_t = memfunc_info_t<&T::act_on_data>::ret_type;
  /// True if the return is void
  static constexpr bool is_void_ret = std::is_same_v<ret_t, void>;
  /// void is a special type because C++ doesn't allow variables of type void(a
  /// mistake). When a variable of type void is needed this can be used instead
  /// and protect uses of the variable with if constexpr.
  using non_void_ret_t = std::conditional_t<is_void_ret, uint32_t, ret_t>;
  using data_t = memfunc_info_t<&T::act_on_data>::args::template get_type<3>;
};

template <typename... Ts> struct service_list_info {
  using base = type_seq<Ts...>;
  template <auto i>
  using get_info_t = service_info<typename base::template get_type<i>>;
  template <typename T>
  static constexpr uint32_t get_index = base::template get_index<T>;
  using info_seq = type_seq<get_info_t<get_index<Ts>>...>;
  using data_seq = type_seq<typename get_info_t<get_index<Ts>>::data_t...>;
  using ret_seq = type_seq<typename get_info_t<get_index<Ts>>::ret_t...>;

  template <typename DT, template <typename...> typename accessorT>
  using service_list_accessor = accessorT<DT, Ts...>;

  /// Dynamically dispatch func with the type in Ts at index idx
  template <typename Func> static void for_any(uint32_t idx, Func func) {
    if constexpr (sizeof...(Ts) > 0) {
      [&]<typename First, typename... Tys>() {
        if (idx == 0) {
          func.template operator()<First>();
          return;
        }
        service_list_info<Tys...>::for_any(idx - 1, func);
      }.template operator()<Ts...>();
    } else {
      TRISYCL_FALLBACK;
    }
  }
};

/// The device_impl stores global information about the device.
/// It is type-erased so it only know void* to memory tile.
struct device_impl_fallback {
  device_impl_fallback() = default;
  device_impl_fallback(int x, int y) { TRISYCL_FALLBACK; }
  void add_storage(hw::position pos, void* storage) { TRISYCL_FALLBACK; }
  void* get_mem(hw::position pos) { TRISYCL_FALLBACK; }
  template <typename ServiceTy> void wait_all(ServiceTy&&) { TRISYCL_FALLBACK; }
};

/// The lock_impl enable interacting with a lock.
struct lock_impl_fallback {
  void acquire() { TRISYCL_FALLBACK; }
  void release() { TRISYCL_FALLBACK; }
  void acquire_with_value(bool val) { TRISYCL_FALLBACK; }
  void release_with_value(bool val) { TRISYCL_FALLBACK; }
};

/// It is useful when executing on hardware to have separate type for locks on
/// device and host. Se here they are separated even if they have the same API.
using device_lock_impl_fallback = lock_impl_fallback;
using host_lock_impl_fallback = lock_impl_fallback;

/// The device_tile_impl enable doing any action that can be done on device by a
/// tile.
struct device_tile_impl_fallback {
  template <typename DeviceImplTy> void init(DeviceImplTy&, hw::position pos) {}
  lock_impl_fallback get_lock(hw::dir d, int i) { TRISYCL_FALLBACK; }
  template <hw::dir d> void* get_mem_addr() { TRISYCL_FALLBACK; }
  void stream_write16(const char* ptr, int stream_dix) { TRISYCL_FALLBACK; }
  void stream_read16(char* ptr, int stream_dix) { TRISYCL_FALLBACK; }
  void cascade_write48(const char* ptr) { TRISYCL_FALLBACK; }
  void cascade_read48(const char* ptr) { TRISYCL_FALLBACK; }
  int x_coord() { TRISYCL_FALLBACK; }
  int y_coord() { TRISYCL_FALLBACK; }
  template <typename T, typename ServiceTy>
  service_info<T>::ret_t perform_service(service_info<T>::data_t data) {
    TRISYCL_FALLBACK;
  }
};

/// The host_tile_impl enable doing any action that can be done from the host to
/// a tile.
struct host_tile_impl_fallback {
  template <typename DeviceImplTy> void init(DeviceImplTy&, hw::position pos) {}
  lock_impl_fallback get_lock(hw::dir d, int i) { TRISYCL_FALLBACK; }
  void* get_mem_addr(hw::dir d) { TRISYCL_FALLBACK; }
  template <typename LambdaTy, typename DeviceTileTy>
  void execute(LambdaTy l, DeviceTileTy& dt) {
    TRISYCL_FALLBACK;
  }
  template <typename AccTy> void register_accessor(const AccTy&) {
    TRISYCL_FALLBACK;
  }
  void notify_has_accessed_mem(void* mem, std::size_t size) {
    TRISYCL_FALLBACK;
  }
  lock_impl_fallback get_lock(int i) {
    TRISYCL_FALLBACK;
    return {};
  }
};

struct device_mem_handle_impl_fallback {
  void memcpy_h2d(generic_ptr<void> p, void* ptr, uint32_t size) {
    TRISYCL_FALLBACK;
  }
  void memcpy_d2h(void* ptr, generic_ptr<void> p, uint32_t size) {
    TRISYCL_FALLBACK;
  }
};

/// Add common load<T> and store<T> built on top of the memcpy_* provided by
/// device_mem_handle_impl
template <typename ImplTy> struct device_mem_handle_adaptor : ImplTy {
  using ImplTy::ImplTy;
  template <typename T> T load(generic_ptr<T> ptr) {
    T res;
    ImplTy::memcpy_d2h(std::addressof(res), ptr, sizeof(T));
    return res;
  }
  template <typename T> void store(generic_ptr<T> ptr, T val) {
    ImplTy::memcpy_h2d(ptr, std::addressof(val), sizeof(T));
  }
};

/// device_accessor_impl is the internal storage of an accessors on the device.
/// it is aligned on 8 such that it has the same layout as it host counterpart.
struct alignas(8) device_accessor_impl {
  device_accessor_impl() = default;
  device_accessor_impl(uint32_t, uint32_t, char*) {}
  device_accessor_impl(uint32_t, uint32_t, char*, uint32_t, uint32_t) {}
  uint32_t size_ = 0;
  hw::dev_ptr<char> data = nullptr;
  char* get_ptr() { return data.get(); }
  const char* get_ptr() const { return data.get(); }
  std::size_t size() { return size_; }
};

struct host_accessor_out_of_line_impl {
  char* data = nullptr;
  uint32_t elem_size;
  uint32_t size;
  uint32_t write_back_start;
  uint32_t write_back_end;
};

/// device_accessor_impl is the internal storage of an accessors on the host.
/// it is aligned on 8 such that it has the same layout as it device
/// counterpart.
struct alignas(8) host_accessor_impl {
  host_accessor_impl() = default;
  host_accessor_impl(const host_accessor_impl& other)
      : host_accessor_impl(*other.impl) {}
  host_accessor_impl(uint32_t s, uint32_t es, char* d)
      : host_accessor_impl(s, es, d, 0, s) {}
  host_accessor_impl(uint32_t s, uint32_t es, char* d, uint32_t write_start,
                     uint32_t write_end)
      : host_accessor_impl(host_accessor_out_of_line_impl {
            (char*)d, es, s, write_start, write_end }) {}
  host_accessor_impl(host_accessor_out_of_line_impl i)
      : impl(std::make_unique<host_accessor_out_of_line_impl>(i)) {}
  std::unique_ptr<host_accessor_out_of_line_impl> impl;
  std::size_t size() { return impl->size; }
  char* get_ptr() { return impl->data; }
  const char* get_ptr() const { return impl->data; }
};

/// This selects the right accessor implementation based on context
#ifdef __SYCL_DEVICE_ONLY__
using accessor_common = device_accessor_impl;
#else
using accessor_common = host_accessor_impl;
#endif

// bool operator==(accessor_common self, accessor_common other) {
//   return self.size == other.size && self.elem_size == other.elem_size &&
//          self.data == other.data;
// }
// bool operator!=(accessor_common self, accessor_common other) {
//   return !(self == other);
// }

/// Validate that accessors have the same layout on host and
static assert_equal<sizeof(accessor_common), 8> check_sizeof_accessor_common;
static assert_equal<alignof(accessor_common), 8> check_alignof_accessor_common;

} // namespace aie::detail

#endif
