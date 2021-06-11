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

#if defined(__SYCL_DEVICE_ONLY__)
#include "acap-intrinsic.h"
#endif

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

/// The ordering matters.
/// It matches with bases addresses order are at address (1 << 17)
/// | (dir << 15). It also match with locks where the id from a tile's view
/// dir << 4 | id.
enum class dir : int8_t {
  south,
  west,
  north,
  east,
  down = south,
  left = west,
  up = north,
  right = east,
};

enum class parity : int8_t {
  west,
  east,
};

constexpr parity get_parity(int X, int Y) { return (Y & 1) ? parity::west : parity::east; }

template <parity par> struct hw_tile_impl;

template <int X, int Y> struct hw_tile : hw_tile_impl<get_parity(X, Y)> {
  using base = hw_tile_impl<get_parity(X, Y)>;
  using dir = typename base::dir;
  using base::get_offset;
  static constexpr pos get_pos(dir d) {
    return pos{X, Y} + base::get_offset(d);
  }
};

template <parity par> struct hw_tile_impl {

  enum dir : int8_t {
    south,
    west,
    north,
    east,
    down = south,
    left = west,
    up = north,
    right = east,
    self = (par == parity::west) ? west : east,
    side = (par == parity::west) ? east : west,
  };

  static constexpr const char *dir_to_str[] = {
      "south/down", (par == parity::west) ? "west/left/self" : "west/left/side", "north/up",
      (par == parity::west) ? "east/right/side" : "east/right/self"};

  static constexpr off get_offset(dir d) {
    switch (d) {
    case south:
      return {-1, 0};
    case north:
      return {1, 0};
    case west:
      if constexpr ((par == parity::west))
        return {0, 0};
      else
        return {0, 1};
    case east:
      if constexpr ((par == parity::west))
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
static constexpr unsigned self_tile_addr(parity par) {
  return ((par == parity::west) ? west_or_self_tile_addr : east_or_self_tile_addr);
}
static constexpr unsigned side_tile_addr(parity par) {
  return (par == parity::west ? east_or_self_tile_addr : west_or_self_tile_addr);
}

/// Size of each tiles in every direction
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

constexpr unsigned graphic_beg_off = log_buffer_end_off;
constexpr unsigned graphic_size = 0x1000;
constexpr unsigned graphic_end_off = graphic_beg_off + graphic_size;

dir get_ptr_dir(uint32_t ptr) { return (dir)((ptr >> 15) & 0x3); }
template <typename T> dir get_ptr_dir(T *p) { return get_ptr_dir((uint32_t)p); }

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)

struct dev_ptr {
  pos p;
  uint32_t offset;
};

/// This will convert a direction into an offset.
constexpr off get_offset(parity par, dir d) {
  switch (d) {
  case dir::south:
    return {0, -1};
  case dir::north:
    return {0, 1};
  case dir::west:
    if (par == parity::west)
      return {0, 0};
    else
      return {-1, 0};
  case dir::east:
    if (par == parity::west)
      return {1, 0};
    else
      return {0, 0};
  }
}

/// this will translate a device representation of a pointer ptr in a tile at
/// position pos. into a tile position and an offset suitable to be used with de
/// xaie::handle.
dev_ptr get_dev_ptr(pos p, uint32_t ptr) {
  dev_ptr out;
  /// extract the offset part of the pointer.
  out.offset = ptr & offset_mask;
  /// extract the direction.
  dir d = get_ptr_dir(ptr);
  out.p = p + get_offset(get_parity(p.x, p.y), d);
  return out;
}

#endif

#ifdef __SYCL_DEVICE_ONLY__
/// The address of the stack will be either in the west or east tile based on
/// the parity of the tile so we can use the address of the stack to figure out
/// the parity. This is what this function does.
parity get_parity_dev() {
  int i;
  /// the 15 low bits are offset within a tile
  /// then bits 16-15 are the index of the tile.
  /// west = 0b01 and east = 0b11
  /// so iff 16s bits of the address of a variable on the stack is set to 1 the
  /// tile is on the east.
  return !((reinterpret_cast<uint32_t>(&i) & (1 << 16))) ? parity::west
                                                         : parity::east;
}

dir get_self_tile_dir() {
  return (get_parity_dev() == parity::west) ? dir::west : dir::east;
}



#endif

struct log_record {
#ifdef __SYCL_DEVICE_ONLY__
  static volatile log_record *get() {
    return (log_record *)(self_tile_addr(get_parity_dev()) + log_buffer_beg_off);
  }
  volatile char* get_data() volatile {
    return data;
  }
#endif
  uint32_t size;
  char data[];
};

}; // namespace hw_mem

#if defined(__SYCL_DEVICE_ONLY__)

namespace hw {

int get_tile_x_cord() {
  return (acap_intr::get_coreid() >> 16) & 0x3f;
}

int get_tile_y_cord() {
  return (acap_intr::get_coreid() & 0xf) - 1;
}
  
}

#endif

} // namespace trisycl::vendor::xilinx::acap

#endif
