#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP

/** \file

    contains hardware specific informations and linker scripts details of
    how the memory is used an partitioned

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <cstdint>

namespace trisycl::vendor::xilinx::acap {

struct off {
  int x;
  int y;
};

struct pos {
  int x;
  int y;
  friend constexpr pos operator+(pos p, off o) {
    return {p.x + o.x, p.y + o.y};
  }
  friend constexpr off operator-(pos p, pos p1) {
    return {p.x - p1.x, p.y - p1.y};
  }
};

constexpr bool is_west(int X, int Y) { return Y & 1; }

template <bool is_west> struct hw_tile_impl;

template <int X, int Y> struct hw_tile : hw_tile_impl<is_west(X, Y)> {
  using base = hw_tile_impl<is_west(X, Y)>;
  using dir = typename base::dir;
  using base::get_offset;
  static constexpr pos get_pos(dir d) {
    return pos{X, Y} + base::get_offset(d);
  }
};

template <bool is_west> struct hw_tile_impl {

  /// The ordering matters.
  /// It matches with bases addresses other tiles with are at address (1 << 17)
  /// | (dir << 15). It also match with locks where the id from a tile's view
  /// dir << 4 | id.
  enum dir : int8_t {
    south,
    west,
    north,
    east,
    down = south,
    left = west,
    up = north,
    right = east,
    self = is_west ? west : east,
    side = is_west ? east : west,
  };

  static constexpr const char *dir_to_str[] = {
      "south/down", is_west ? "west/left/self" : "west/left/side", "north/up",
      is_west ? "east/right/side" : "east/right/self"};

  static constexpr off get_offset(dir d) {
    switch (d) {
    case south:
      return {-1, 0};
    case north:
      return {1, 0};
    case west:
      if constexpr (is_west)
        return {0, 0};
      else
        return {0, 1};
    case east:
      if constexpr (is_west)
        return {0, -1};
      else
        return {0, 0};
    }
  }
};

namespace hw_mem {
/// hardware specific details. should not change.

/// Base address of one tile and its neighbors.
constexpr unsigned south_tile_addr = 0x20000;        // (1 << 17) | (0 << 15)
constexpr unsigned west_or_self_tile_addr = 0x28000; // (1 << 17) | (1 << 15)
constexpr unsigned north_tile_addr = 0x30000;        // (1 << 17) | (2 << 15)
constexpr unsigned east_or_self_tile_addr = 0x38000; // (1 << 17) | (3 << 15)

/// Base address a tile viewed from itself.
static constexpr unsigned self_tile_addr(unsigned X, unsigned Y) {
  return ((Y & 1) ? west_or_self_tile_addr : east_or_self_tile_addr);
}
static constexpr unsigned side_tile_addr(unsigned X, unsigned Y) {
  return ((Y & 1) ? east_or_self_tile_addr : west_or_self_tile_addr);
}

constexpr unsigned tile_size = 0x8000;
constexpr unsigned offset_mask = tile_size - 1;
constexpr unsigned base_addr_mask = ~offset_mask;

/// linker script details. any change here need to be reflected in the linker
/// script and vice versa.
/// This will probably be made dynamic in the future to allow more efficient
/// memory usage
/// Variable below are offsets so the need to be used in combination with base
/// tile addresses. when viewed from the host tile base address is 0 so
/// offsets can be used alone when accessing from the host.
constexpr unsigned stack_beg_off = 0x0;
constexpr unsigned stack_size = 0x1000;
constexpr unsigned stack_end_off = stack_beg_off + stack_size;

constexpr unsigned args_beg_off = stack_end_off;
constexpr unsigned args_size = 0x2000;
constexpr unsigned args_end_off = args_beg_off + args_size;

constexpr unsigned tile_mem_beg_off = args_end_off;
constexpr unsigned tile_mem_size = 0x2000;
constexpr unsigned tile_mem_end_off = tile_mem_beg_off + tile_mem_size;

constexpr unsigned log_buffer_beg_off = tile_mem_end_off;
constexpr unsigned log_buffer_size = 0x1000;
constexpr unsigned log_buffer_end_off = log_buffer_beg_off + log_buffer_size;

struct log_record {
  static log_record *get(unsigned X, unsigned Y) {
    return (log_record *)(self_tile_addr(X, Y) + log_buffer_beg_off);
  }
  uint32_t size;
  // uint32_t host_idx;
  char* get_data() {
    return (char*)(this + 1);
  }
};

}; // namespace hw_mem

} // namespace trisycl::vendor::xilinx::acap

#endif
