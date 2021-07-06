#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP

/** \file

    Contains hardware-specific information and linker script details of
    how the memory is used an partitioned

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <cstring>

#if defined(__SYCL_DEVICE_ONLY__)
#include "acap-intrinsic.h"
#endif

namespace trisycl::vendor::xilinx::acap {

namespace hw {

/// Represent of offset between 2 tile.
struct offset {
  int x;
  int y;
};

/// Represent the position of a tile
struct position {
  int x;
  int y;
  friend constexpr position operator+(position p, offset o) {
    return {p.x + o.x, p.y + o.y};
  }
  friend constexpr offset operator-(position p, position p1) {
    return {p.x - p1.x, p.y - p1.y};
  }
};

/// The ordering  and specific values of elements matters in the following enum.
/// It matches with bases addresses order. neighbouring tiles of a core are at
/// address: (1 << 17) | (dir << 15) from the perspective of tha core. It also
/// match with locks where the id for a core's view is: dir << 4 | id.
enum class dir : int8_t {
  south,
  west,
  north,
  east,
  // Add some aliases
};

/// represent parity of a tile
enum class parity : int8_t {
  west,
  east,
};

/// get the parity of tile at position p
constexpr parity get_parity(position p) {
  return (p.y & 1) ? parity::west : parity::east;
}

template <parity par> struct hw_tile_impl;

template<int X, int Y> struct hw_tile : hw_tile_impl<get_parity({X, Y})> {
  using base = hw_tile_impl<get_parity({X, Y})>;
  using dir = typename base::dir;
  using base::get_offset;
  static constexpr position get_pos(dir d) {
    return position{X, Y} + base::get_offset(d);
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

  static constexpr offset get_offset(dir d) {
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

/// hardware specific details.

/// Base address of memory modules around the tile.
constexpr uint32_t south_tile_addr = 0x20000;        // (1 << 17) | (0 << 15)
constexpr uint32_t west_or_self_tile_addr = 0x28000; // (1 << 17) | (1 << 15)
constexpr uint32_t north_tile_addr = 0x30000;        // (1 << 17) | (2 << 15)
constexpr uint32_t east_or_self_tile_addr = 0x38000; // (1 << 17) | (3 << 15)

constexpr uint32_t get_base_addr(dir d) {
  return (1 << 17) | ((uint32_t)d << 15);
}

/// Base address of the tile viewed from itself
constexpr uint32_t self_tile_addr(parity par) {
  return ((par == parity::west) ? west_or_self_tile_addr
                                : east_or_self_tile_addr);
}

/// Base address of neighbouring east or west tile
constexpr uint32_t side_tile_addr(parity par) {
  return (par == parity::west ? east_or_self_tile_addr : west_or_self_tile_addr);
}

/// Size of the memory module of every tile.
/// Not the whole memory just its own memory
constexpr uint32_t tile_size = 0x8000;
constexpr uint32_t offset_mask = tile_size - 1;
constexpr uint32_t base_addr_mask = ~offset_mask;

/// linker script details. any change here need to be reflected in the linker
/// script and vice versa.
/// This will probably be made dynamic in the future to allow more efficient
/// memory usage
/// Variable below are offsets so the need to be used in combination with base
/// tile addresses. when viewed from the host tile base address is 0 so
/// offsets can be used alone when accessing from the host.
constexpr uint32_t stack_beg_off = 0x0;
constexpr uint32_t stack_size = 0x1000;
constexpr uint32_t stack_end_off = stack_beg_off + stack_size;

/// contains the "arguments", this is where the kernel object will be
/// placed.
constexpr uint32_t args_beg_off = stack_end_off;
constexpr uint32_t args_size = 0x1400;
constexpr uint32_t args_end_off = args_beg_off + args_size;

/// contains the memory modules that are shared across tiles.
/// Technically all section are shared but sections other then this one should
/// not be access by your neighbors.
constexpr uint32_t tile_mem_beg_off = args_end_off;
constexpr uint32_t tile_mem_size = 0x4000;
constexpr uint32_t tile_mem_end_off = tile_mem_beg_off + tile_mem_size;

/// contains the log_record and the log buffer that follows it.
constexpr uint32_t log_buffer_beg_off = tile_mem_end_off;
constexpr uint32_t log_buffer_size = 0x800;
constexpr uint32_t log_buffer_end_off = log_buffer_beg_off + log_buffer_size;

/// contains the graphics_record.
constexpr uint32_t graphic_beg_off = log_buffer_end_off;
constexpr uint32_t graphic_size = 0x400;
constexpr uint32_t graphic_end_off = graphic_beg_off + graphic_size;

/// Offset of the last dedicated section
constexpr uint32_t last_end_off = graphic_end_off;

/// The minimum size we want to keep for globabl variables
constexpr uint32_t min_global_variable_size = 0x800;

static_assert(last_end_off + min_global_variable_size <= tile_size, "sections are using more memory than is available");

/// determine the direction of the memory module pointed into by a pointer on
/// device.
dir get_ptr_direction(uint32_t ptr) { return (dir)((ptr >> 15) & 0x3); }
template<typename T> dir get_ptr_direction(T *p) {
  return get_ptr_direction((uint32_t)p);
}

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)

/// an address in memory on any tile of the device.
struct dev_ptr {
  /// The position of the tile this pointer is into
  position p;
  /// Offset within the memory module of the tile.
  uint32_t offset;
};

/// Convert a direction into an offset
constexpr offset get_offset(parity par, dir d) {
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
/** Translate the device representation of a pointer into something
    usable by/with xaie::handle and libXAiengine

    \param[in] p is the position of the tile
    \param[in] ptr is a local pointer inside the tile
    \return a dev_ptr and aggregation of the position of the memory module/tile
   and the offset with that memory module
 */
dev_ptr get_dev_ptr(position p, uint32_t ptr) {
  dev_ptr out;
  /// Extract the offset part of the pointer
  out.offset = ptr & offset_mask;
  /// Extract the part direction.
  dir d = get_ptr_direction(ptr);
  out.p = p + get_offset(get_parity(p), d);
  return out;
}

#endif

#ifdef __SYCL_DEVICE_ONLY__
/// The address of the stack will be either in the west or east memory module
/// based on the parity of the memory module so we can use the address of the
/// stack to figure out the parity. This is what this function does.
parity get_parity_dev() {
  int i;
  /// The 15 low bits are the offset within a tile memory module.
  /// Bits 16-15 are the index of the tile memory module,
  /// West = 0b01 and East = 0b11
  /// so if and only if the bit 16 of the address of a variable on the stack is
  /// set to 1, the memory module is on the East of the tile.
  return ((reinterpret_cast<uint32_t>(&i) & (1 << 16))) ? parity::east
                                                        : parity::west;
}

/// Get the direction of the memory module of the tile executing this.
dir get_own_memory_module_dir() {
  return (get_parity_dev() == parity::west) ? dir::west : dir::east;
}

#endif

struct log_record {
#ifdef __SYCL_DEVICE_ONLY__
  /// For device only

  /// get the address the log_record
  /// It is marked volatile because the data can be read by the host
  static volatile log_record *get() {
    return (log_record *)(self_tile_addr(get_parity_dev()) +
                          log_buffer_beg_off);
  }
  /// get the address of the buffer to write logs into, it is volatile because
  /// the host can read it.
  volatile char *get_data() volatile { return data; }
#endif
  uint32_t host_emit_count;
  uint32_t size;
  char data[];
};

#if defined(__SYCL_DEVICE_ONLY__)

/// get the X coordinate in the acap model
int get_tile_x_coordinate() { return (acap_intr::get_coreid() >> 16) & 0x7f; }

/// get the Y coordinate in the acap model
int get_tile_y_coordinate() {
  /// the -1 is to skip shim row
  return (acap_intr::get_coreid() & 0x1f) - 1;
}

uint32_t strlen(const char *ptr) {
  uint32_t i = 0;
  while (ptr[i])
    i++;
  return i;
}

#endif

} // namespace hw

} // namespace trisycl::vendor::xilinx::acap

#endif
