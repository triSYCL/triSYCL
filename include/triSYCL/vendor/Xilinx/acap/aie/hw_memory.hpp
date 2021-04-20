#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP

/** \file

    contains hardware specific informations and linker scripts details of
    how the memory is used an partitioned

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::vendor::xilinx::acap {

struct hw_mem {
  /// hardware specific details. should not change.

  /// Base address of one tile and its neighbors.
  static constexpr unsigned south_tile_addr = 0x20000;
  static constexpr unsigned west_or_self_tile_addr = 0x28000;
  static constexpr unsigned north_tile_addr = 0x30000;
  static constexpr unsigned east_or_self_tile_addr = 0x38000;

  /// Base address a tile viewed from itself.
  template <int X, int Y>
  static constexpr unsigned self_tile_addr = ((Y & 1) ? west_or_self_tile_addr
                                                      : east_or_self_tile_addr);
  template <int X, int Y>
  static constexpr unsigned side_tile_addr = ((Y & 1) ? east_or_self_tile_addr
                                                      : west_or_self_tile_addr);

  static constexpr unsigned tile_size = 0x8000;
  static constexpr unsigned offset_mask = tile_size - 1;
  static constexpr unsigned base_addr_mask = ~offset_mask;

  /// linker script details. any change here need to be reflected in the linker
  /// script and vice versa.
  /// This will probably be made dynamic in the future to allow more efficient
  /// memory usage
  /// Variable below are offsets so the need to be used in combination with base
  /// tile addresses. when viewed from the host tile base address is 0 so
  /// offsets can be used alone when accessing from the host.
  static constexpr unsigned stack_beg_off = 0x0;
  static constexpr unsigned stack_size = 0x1000;
  static constexpr unsigned stack_end_off = stack_beg_off + stack_size;

  static constexpr unsigned args_beg_off = stack_end_off;
  static constexpr unsigned args_size = 0x2000;
  static constexpr unsigned args_end_off = args_beg_off + args_size;

  static constexpr unsigned tile_mem_beg_off = args_end_off;
  static constexpr unsigned tile_mem_size = 0x2000;
  static constexpr unsigned tile_mem_end_off =
      tile_mem_beg_off + tile_mem_size;
};

} // namespace trisycl::vendor::xilinx::acap

#endif
