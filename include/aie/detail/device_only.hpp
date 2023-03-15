
#ifndef AIE_DETAIL_DEVICE_ONLY_HPP
#define AIE_DETAIL_DEVICE_ONLY_HPP

#if defined(__ACAP_EMULATION___) || !defined(__SYCL_DEVICE_ONLY__)
#error "should only be used on the host side of hardware execution"
#endif

#include "common.hpp"
#include "device_and_host.hpp"
#include "acap-intrinsic.h"

namespace aie::detail {

using device_impl = device_impl_fallback;

struct device_tile_impl : device_tile_impl_fallback {
  lock_impl_fallback get_lock(hw::dir d, int i) { TRISYCL_FALLBACK; }
  template <hw::dir d> void* get_mem_addr() {
    return hw::get_object<void>(hw::offset_table::get_tile_mem_begin_offset(), d);
  }
  void stream_write16(const char* ptr, int stream_dix) {
    acap_intr::stream_write16(ptr, stream_dix);
  }
  void stream_read16(char* ptr, int stream_dix) {
    acap_intr::stream_read16(ptr, stream_dix);
  }
  void cascade_write48(const char* ptr) {
    acap_intr::cstream_write48(ptr);
  }
  void cascade_read48(char* ptr) {
    acap_intr::cstream_read48(ptr);
  }
  int x_coord() { return hw::get_tile_x_coordinate(); }
  int y_coord() { return hw::get_tile_y_coordinate(); }
};

} // namespace aie::detail

#endif
