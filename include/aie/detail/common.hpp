
#ifndef AIE_DETAIL_COMMON_HPP
#define AIE_DETAIL_COMMON_HPP

#include "hardware.hpp"
#include "utils.hpp"

#define TRISYCL_FALLBACK                                                       \
  do {                                                                         \
    assert(false && "fallback");                                               \
    __builtin_unreachable();                                                   \
  } while (0)
#define TRISYCL_UNIMPLEMENTED                                                  \
  do {                                                                         \
    assert(false && "unimplementatble");                                       \
    __builtin_unreachable();                                                   \
  } while (0)

#ifdef __SYCL_DEVICE_ONLY__
#define __SYCL_TYPE(x) [[__sycl_detail__::sycl_type(x)]]
#else
#define __SYCL_TYPE(x)
#endif

namespace aie::detail {

struct illegal_to_access {};

struct device_impl_fallback {
  device_impl_fallback() = default;
  device_impl_fallback(int x, int y) { TRISYCL_FALLBACK; }
  void add_storage(hw::position pos, void* storage) { TRISYCL_FALLBACK; }
  void* get_mem(hw::position pos) { TRISYCL_FALLBACK; }
  void wait_all() { TRISYCL_FALLBACK; }
};

struct lock_impl_fallback {
  void acquire() { TRISYCL_FALLBACK; }
  void release() { TRISYCL_FALLBACK; }
  void acquire_with_value(bool val) { TRISYCL_FALLBACK; }
  void release_with_value(bool val) { TRISYCL_FALLBACK; }
};

using device_lock_impl_fallback = lock_impl_fallback;
using host_lock_impl_fallback = lock_impl_fallback;

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

static assert_equal<sizeof(accessor_common), 16> check_sizeof_accessor_common;
static assert_equal<alignof(accessor_common), 8> check_alignof_accessor_common;

} // namespace aie::detail

#endif
