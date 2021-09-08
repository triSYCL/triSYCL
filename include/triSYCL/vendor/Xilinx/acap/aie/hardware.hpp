#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_HW_MEMORY_HPP

/** \file

    Contains hardware-specific information and linker script details of
    how the memory is used an partitioned

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <cstring>

#if defined(__SYCL_DEVICE_ONLY__)
#include "acap-intrinsic.h"
#endif

namespace trisycl::vendor::xilinx::acap {

namespace hw {

/// The ordering  and specific values of elements matters in the following enum.
/// It matches with bases addresses order. neighbouring tiles of a core are at
/// address: (1 << 17) | (dir << 15) from the perspective of tha core. It also
/// match with locks where the id for a core's view is: dir << 4 | id.
enum class dir : int8_t {
  south,
  west,
  north,
  east,
};

/// represent parity of a tile
enum class parity : int8_t {
  west,
  east,
};

/// Represent of offset between 2 tile.
struct offset {
  int x;
  int y;
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
  /// get the parity of tile at position p
  constexpr parity get_parity() const { return (y & 1) ? parity::west : parity::east; }
  constexpr position moved(dir d) const { return *this + get_offset(get_parity(), d); }
};

constexpr parity get_parity(position p) { return p.get_parity(); }

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
constexpr uint32_t stack_begin_offset = 0x0;
constexpr uint32_t stack_size = 0x1000;
constexpr uint32_t stack_end_offset = stack_begin_offset + stack_size;

/// contains the "arguments", this is where the kernel object will be
/// placed.
constexpr uint32_t args_begin_offset = stack_end_offset;
constexpr uint32_t args_size = 0x1400;
constexpr uint32_t args_end_offset = args_begin_offset + args_size;

/// contains the memory modules that are shared across tiles.
/// Technically all section are shared but sections other then this one should
/// not be access by your neighbors.
constexpr uint32_t tile_mem_begin_offset = args_end_offset;
constexpr uint32_t tile_mem_size = 0x4000;
constexpr uint32_t tile_mem_end_offset = tile_mem_begin_offset + tile_mem_size;

/// contains the graphics_record.
constexpr uint32_t rpc_record_begin_offset = tile_mem_end_offset;
constexpr uint32_t rpc_record_size = 0x1000;
constexpr uint32_t rpc_record_end_offset = rpc_record_begin_offset + rpc_record_size;

/// Offset of the last dedicated section
constexpr uint32_t last_end_offset = rpc_record_end_offset;

/// The minimum size we want to keep for globabl variables
constexpr uint32_t min_global_variable_size = 0x800;

static_assert(last_end_offset + min_global_variable_size <= tile_size, "sections are using more memory than is available");

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

#if defined(__SYCL_DEVICE_ONLY__)

/// get the X coordinate in the acap model
int get_tile_x_coordinate() {
  return (acap_intr::get_coreid() >> 16) & 0b1111111;
}

/// get the Y coordinate in the acap model
int get_tile_y_coordinate() {
  /// the -1 is to skip shim row
  return (acap_intr::get_coreid() & 0b11111) - 1;
}

uint32_t strlen(const char *ptr) {
  uint32_t i = 0;
  while (ptr[i])
    i++;
  return i;
}

int pow(int i, int p) {
  int res = 1;
  for (; p > 0; p--)
    res *= i;
  return res;
}

#endif

/// This is function is very similar to the C++20 std::bit_cast but it accepts
/// bitcasting between types of diffrent sizes. when sizeof(To) > sizeof(From)
/// bit after sizeof(From) will be 0. when sizeof(From) > sizeof(To), the result
/// will only contain part of the original object.
template <typename To, typename From>
inline To bit_cast(const From &from) noexcept {
  To to;
  std::memset(&to, 0, sizeof(To));
  std::memcpy(&to, &from, std::min(sizeof(To), sizeof(From)));
  return to;
}

/// stable_pointer is a representation of a device pointer that has the same
/// layout between the host and the device. Ths issue with this approach is that
/// applying a + or - to a stable_pointer doesn't result in the same value on
/// the host and the device.
#if defined(__SYCL_DEVICE_ONLY__)
template<typename T>
using stable_pointer = T*;
static_assert(sizeof(stable_pointer<void>) == sizeof(std::uint32_t));
#else
template<typename T>
using stable_pointer = uint32_t;
#endif

} // namespace hw

} // namespace trisycl::vendor::xilinx::acap

#endif
