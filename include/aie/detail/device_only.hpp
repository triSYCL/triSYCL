
#ifndef AIE_DETAIL_DEVICE_ONLY_HPP
#define AIE_DETAIL_DEVICE_ONLY_HPP

#if defined(__ACAP_EMULATION___) || !defined(__SYCL_DEVICE_ONLY__)
#error "should only be used on the host side of hardware execution"
#endif

#include "acap-intrinsic.h"
#include "common.hpp"
#include "device_and_host.hpp"

namespace aie::detail {

struct device_lock_impl {
 private:
  int id;

 public:
  device_lock_impl(hw::dir d, int i)
      : id { (detail::underlying_value(hw::resolved_dir(d)) * 16) + i } {
    assert(i < 16);
    assert(id < 64);
  }
  void acquire() { acap_intr::acquire(id); }
  void release() { acap_intr::release(id); }
  void acquire_with_value(bool val) { acap_intr::acquire(id, val); }
  void release_with_value(bool val) { acap_intr::release(id, val); }
};

using host_lock_impl = lock_impl_fallback;
using device_impl = device_impl_fallback;

struct device_tile_impl : device_tile_impl_fallback {
  template <hw::dir d> void* get_mem_addr() {
    return hw::get_object<void>(hw::offset_table::get_tile_mem_begin_offset(),
                                d);
  }
  void stream_write16(const char* ptr, int stream_dix) {
    acap_intr::stream_write16(ptr, stream_dix);
  }
  void stream_read16(char* ptr, int stream_dix) {
    acap_intr::stream_read16(ptr, stream_dix);
  }
  void cascade_write48(const char* ptr) { acap_intr::cstream_write48(ptr); }
  void cascade_read48(char* ptr) { acap_intr::cstream_read48(ptr); }
  int x_coord() { return hw::get_tile_x_coordinate(); }
  int y_coord() { return hw::get_tile_y_coordinate(); }
  device_lock_impl get_lock(hw::dir d, int i) { return device_lock_impl(d, i); }
};

} // namespace aie::detail

#endif
