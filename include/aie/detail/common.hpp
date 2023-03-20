
#ifndef AIE_DETAIL_COMMON_HPP
#define AIE_DETAIL_COMMON_HPP

#include "hardware.hpp"
#include "utils.hpp"

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

namespace aie::detail {

/// Used as the type of a memory tile that should not be accessed.
struct out_of_bounds {};

/// The device_impl stores global information about the device.
/// It is type-erased so it only know void* to memory tile.
struct device_impl_fallback {
  device_impl_fallback() = default;
  device_impl_fallback(int x, int y) { TRISYCL_FALLBACK; }
  void add_storage(hw::position pos, void* storage) { TRISYCL_FALLBACK; }
  void* get_mem(hw::position pos) { TRISYCL_FALLBACK; }
  void wait_all() { TRISYCL_FALLBACK; }
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

/// device_accessor_impl is the internal storage of an accessors on the device.
/// it is aligned on 8 such that it has the same layout as it host counterpart.
struct alignas(8) device_accessor_impl {
  device_accessor_impl() = default;
  device_accessor_impl(unsigned, unsigned, void*) {}
  unsigned size = 0;
  unsigned elem_size = 0;
  hw::dev_ptr<char> data = nullptr;
  char* get_ptr() { return data.get(); }
  const char* get_ptr() const { return data.get(); }
#ifdef __SYCL_DEVICE_ONLY__
  uint32_t padding = 0;
#endif
};

/// device_accessor_impl is the internal storage of an accessors on the host.
/// it is aligned on 8 such that it has the same layout as it device counterpart.
struct alignas(8) host_accessor_impl {
  host_accessor_impl() = default;
  host_accessor_impl(unsigned s, unsigned es, void* d)
      : size(s)
      , elem_size(es)
      , data((char*)d) {}
  unsigned size = 0;
  unsigned elem_size = 0;
  char* data = nullptr;
  char* get_ptr() { return data; }
  const char* get_ptr() const { return data; }
};

/// This selects the right accessor implementation based on context
#ifdef __SYCL_DEVICE_ONLY__
using accessor_common = device_accessor_impl;
#else
using accessor_common = host_accessor_impl;
#endif

bool operator==(accessor_common self, accessor_common other) {
  return self.size == other.size && self.elem_size == other.elem_size &&
         self.data == other.data;
}
bool operator!=(accessor_common self, accessor_common other) {
  return !(self == other);
}

/// Validate that accessors have the same layout on host and
static assert_equal<sizeof(accessor_common), 16> check_sizeof_accessor_common;
static assert_equal<alignof(accessor_common), 8> check_alignof_accessor_common;

} // namespace aie::detail

#endif
