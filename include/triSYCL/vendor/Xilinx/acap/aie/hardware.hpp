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
#include <cassert>

#ifdef __SYCL_XILINX_AIE__
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

/// Convert a into an offset ignoring parity.
/// The Geography calculation already take in account parity so when giving
/// offset to those API we need to use this instead of get_offset
constexpr offset get_simple_offset(dir d) {
  switch (d) {
  case dir::south:
    return {0, -1};
  case dir::north:
    return {0, 1};
  case dir::west:
    return {-1, 0};
  case dir::east:
    return {1, 0};
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
constexpr uint32_t stack_size = 0x800;
constexpr uint32_t stack_end_offset = stack_begin_offset + stack_size;

/// contains the "arguments", this is where the kernel object will be
/// placed.
constexpr uint32_t args_begin_offset = stack_end_offset;
constexpr uint32_t args_size = 0x1000;
constexpr uint32_t args_end_offset = args_begin_offset + args_size;

/// contains the memory modules that are shared across tiles.
/// Technically all section are shared but sections other then this one should
/// not be access by your neighbors.
constexpr uint32_t tile_mem_begin_offset = args_end_offset;
constexpr uint32_t tile_mem_size = 0x4000;
constexpr uint32_t tile_mem_end_offset = tile_mem_begin_offset + tile_mem_size;

/// contains the RPC system.
constexpr uint32_t rpc_record_begin_offset = tile_mem_end_offset;
constexpr uint32_t rpc_record_size = 56;
constexpr uint32_t rpc_record_end_offset = rpc_record_begin_offset + rpc_record_size;

// This is the heap that is used by dynamic allocations and the allocator's bookeeping.
constexpr uint32_t heap_begin_offset = rpc_record_end_offset;
constexpr uint32_t heap_size = 0x1000;
constexpr uint32_t heap_end_offset = heap_begin_offset + heap_size;

/// Offset of the last dedicated section
constexpr uint32_t last_end_offset = heap_end_offset;

/// The minimum size we want to keep for global variables
constexpr uint32_t min_global_variable_size = 0x800;

static_assert(last_end_offset + min_global_variable_size <= tile_size, "sections are using more memory than is available");

/// determine the direction of the memory module pointed into by a pointer on
/// device.
dir get_ptr_direction(uint32_t ptr) { return (dir)((ptr >> 15) & 0x3); }
template<typename T> dir get_ptr_direction(T *p) {
  return get_ptr_direction((uint32_t)p);
}

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
/// bitcasting between types of diffrent sizes. When sizeof(To) > sizeof(From)
/// bit after sizeof(From) will be 0. When sizeof(From) > sizeof(To), the result
/// will only contain part of the original object.
template <typename To, typename From>
inline To bit_cast(const From &from) noexcept {
  To to;
  std::memset(&to, 0, sizeof(To));
  std::memcpy(&to, &from, std::min(sizeof(To), sizeof(From)));
  return to;
}

/// stable_pointer is a representation of a device pointer that has the same
/// layout between the host and the device. This also contains function to
/// manipulate pointer representations and pointer arithmetic
template<typename T>
struct dev_ptr {
#if defined(__SYCL_DEVICE_ONLY__)
/// On device a pointer is just a pointer
T* ptr;
static T* add(T* ptr, std::ptrdiff_t off) { return ptr + off; }
T* get() { return ptr; }
#else
/// On the host a device pointer is 
std::uint32_t ptr;
static std::uint32_t add(std::uint32_t ptr, std::ptrdiff_t off) {
  /// to match pointer arithmetic we need to multiply by sizeof(T)
  return ptr + off * sizeof(T);
}
T *get() {
  assert(false && "should never be executed on the host");
  return nullptr;
}
#endif
  void set(uint32_t val) { ptr = (decltype(dev_ptr::ptr))val; }
  void set(T* val) { ptr = (decltype(dev_ptr::ptr))val; }

  /// There is not implementation of operator* because it is invalid on dev_ptr<void>.
  T* operator->() { return get(); }
  operator bool() { return *this != dev_ptr(nullptr); }

  /// Pointer arithmetic
  dev_ptr operator+(std::ptrdiff_t off) { return {add(ptr, off)}; }
  dev_ptr operator-(std::ptrdiff_t off) { return {add(ptr, -off)}; }
  std::ptrdiff_t operator-(dev_ptr other) { return ptr - other.ptr; }
  dev_ptr& operator++() { ptr = add(ptr, 1); return *this; }
  dev_ptr operator++(int) { dev_ptr old = *this; *this++; return old; }
  dev_ptr& operator--() { ptr = add(ptr, -1); return *this; }
  dev_ptr operator--(int) { dev_ptr old = *this; *this--; return old; }

  /// Pointer comparaison
  bool operator==(dev_ptr other) const { return ptr == other.ptr; }
  bool operator!=(dev_ptr other) const { return ptr != other.ptr; }
  bool operator<(dev_ptr other) const { return ptr < other.ptr; }
  bool operator<=(dev_ptr other) const { return ptr <= other.ptr; }
  bool operator>(dev_ptr other) const { return ptr > other.ptr; }
  bool operator>=(dev_ptr other) const { return ptr >= other.ptr; }

  /// Pointer assignment
  dev_ptr& operator+=(std::ptrdiff_t off) { *this = *this + off; return *this; }
  dev_ptr& operator-=(std::ptrdiff_t off) { *this = *this - off; return *this; }
  dev_ptr& operator=(T* p) { set(p); return *this; }
  dev_ptr& operator=(std::nullptr_t) { set(nullptr); return *this; }

  /// Manipulating representation
  dir get_dir() { return get_ptr_direction(ptr); }
  std::uint32_t get_offset() { return ((std::uint32_t)ptr) & offset_mask; }
  static constexpr dev_ptr create(hw::dir d, uint32_t offset) {
    return {(decltype(dev_ptr::ptr))(get_base_addr(d) + offset)};
  }
};

/// Validate that the layout is the same on device and host
static_assert(sizeof(dev_ptr<void>) == sizeof(std::uint32_t) &&
              sizeof(dev_ptr<void>) == 4);

} // namespace hw

} // namespace trisycl::vendor::xilinx::acap

#endif
