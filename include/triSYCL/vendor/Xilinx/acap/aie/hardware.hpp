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
  self = -1
};

void constexpr assert_is_resolved_dir(dir d) {
  assert(d >= dir::south && d <= dir::east);
}

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
constexpr inline offset get_offset(parity par, dir d) {
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
  case dir::self:
    return {0, 0};
  }
}

/// Convert a into an offset ignoring parity.
/// The Geography calculation already take in account parity so when giving
/// offset to those API we need to use this instead of get_offset
constexpr inline offset get_simple_offset(dir d) {
  switch (d) {
  case dir::south:
    return {0, -1};
  case dir::north:
    return {0, 1};
  case dir::west:
    return {-1, 0};
  case dir::east:
    return {1, 0};
  case dir::self:
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
constexpr parity get_parity(int x, int y) { return get_parity(position{x, y}); }

/// hardware specific details.

/// Base address of memory modules around the tile.
constexpr uint32_t south_tile_addr = 0x20000;        // (1 << 17) | (0 << 15)
constexpr uint32_t west_or_self_tile_addr = 0x28000; // (1 << 17) | (1 << 15)
constexpr uint32_t north_tile_addr = 0x30000;        // (1 << 17) | (2 << 15)
constexpr uint32_t east_or_self_tile_addr = 0x38000; // (1 << 17) | (3 << 15)

/// Size of the memory module of every tile.
/// Not the whole memory just its own memory
constexpr uint32_t tile_size = 0x8000;
constexpr uint32_t offset_mask = tile_size - 1;
constexpr uint32_t base_addr_mask = ~offset_mask;

/// determine the direction of the memory module pointed into by a pointer on
/// device.
constexpr dir get_ptr_direction(uint32_t ptr) { return (dir)((ptr >> 15) & 0x3); }
template<typename T> dir get_ptr_direction(T *p) {
  return get_ptr_direction((uint32_t)p);
}

/// The address of the stack will be either in the west or east memory module
/// based on the parity of the memory module so we can use the address of the
/// stack to figure out the parity. This is what this function does.
inline parity get_parity_dev() {
#ifdef __SYCL_DEVICE_ONLY__
  int i;
  /// The 15 low bits are the offset within a tile memory module.
  /// Bits 16-15 are the index of the tile memory module,
  /// West = 0b01 and East = 0b11
  /// so if and only if the bit 16 of the address of a variable on the stack is
  /// set to 1, the memory module is on the East of the tile.
  return ((reinterpret_cast<uint32_t>(&i) & (1 << 16))) ? parity::east
                                                        : parity::west;
#else
  assert(false && "this can only be used on device");
  return parity::east;
#endif
}

/// Get the direction of the memory module based on the parity
constexpr dir get_self_dir(parity p) {
  return (p == parity::west) ? dir::west : dir::east;
}

/// Get the direction of the memory module of the tile executing this.
inline dir get_self_dir() { return get_self_dir(get_parity_dev()); }

inline dir resolved_dir(dir d) {
  if (d == dir::self)
    return get_self_dir();
  return d;
}

constexpr uint32_t get_base_addr(dir d) {
  return (1 << 17) | ((uint32_t)d << 15);
}

/// Base address of the tile viewed from itself
constexpr uint32_t self_tile_addr(parity par) {
  return ((par == parity::west) ? west_or_self_tile_addr
                                : east_or_self_tile_addr);
}

/// Base address of the tile viewed from itself
inline uint32_t self_tile_addr() {
  return ((get_parity_dev() == parity::west) ? west_or_self_tile_addr
                                             : east_or_self_tile_addr);
}

template <typename T>
T *get_object(uint32_t offset, hw::dir d = hw::dir::self) {
#ifndef __SYCL_DEVICE_ONLY__
  assert(false && "should only be called on device");
#endif
  return reinterpret_cast<T *>(get_base_addr(resolved_dir(d)) + offset);
}

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

/// The chess backend has a codegen bug that prevent standard conditional based
/// abs from working. here is a bit manipulation implementation of abs.
int abs(int a) {
  /// The complier is smart enough to figure out that this is a normal abs that
  /// will get misscompiled. and will turn it into an abs if the is not
  /// volatile.
  volatile int const mask = a >> (sizeof(decltype(a)) * 8 - 1);

  return (a + mask) ^ mask;
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
  T* ptr = nullptr;
  static T* add(T* ptr, std::ptrdiff_t off) { return ptr + off; }
  T* get() const { return ptr; }
  uint32_t get_int() const { return (uint32_t)ptr; }
#else
  /// On the host a device pointer is
  std::uint32_t ptr = 0;
  static std::uint32_t add(std::uint32_t ptr, std::ptrdiff_t off) {
    /// to match pointer arithmetic we need to multiply by sizeof(T)
    return ptr + off * sizeof(T);
  }
  T* get() const {
    assert(false && "should never be executed on the host");
    return nullptr;
  }
  uint32_t get_int() const { return ptr; }
#endif
  void set(uint32_t val) { ptr = (decltype(dev_ptr::ptr))val; }
  void set(T* val) { ptr = (decltype(dev_ptr::ptr))val; }

  dev_ptr() = default;
  explicit dev_ptr(std::nullptr_t) : dev_ptr() {}
  explicit dev_ptr(uint32_t offset)
      : dev_ptr() {
    set(offset);
  }

  /// This function will be SFNIAE out for T == void. without causing hard
  /// error on instantiation of the class. T2 should never be specified by the
  /// user.
  template <typename T2 = T, typename std::enable_if<
                                 !std::is_same<T2, void>::value, int>::type = 0>
  T2& operator*() {
    static_assert(std::is_same<T2, T>::value,
                  "T2 should be not be specifier by the user");
    return *get();
  }
  template <typename T2 = T, typename std::enable_if<
                                 !std::is_same<T2, void>::value, int>::type = 0>
  T2& operator[](std::size_t index) {
    return get()[index];
  }
  T* operator->() { return get(); }
  operator bool() { return get_int() != 0; }
  template <typename OtherT> explicit operator dev_ptr<OtherT>() {
    return {(OtherT*)ptr};
  }
  template <typename OtherT> explicit operator OtherT*() {
    return (OtherT*)get();
  }

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
  dev_ptr& operator=(std::nullptr_t) { set((uint32_t)0); return *this; }

  /// Manipulating representation
  dir get_dir() { return get_ptr_direction(ptr); }
  void set_offset(uint32_t offset) {
    assert((offset & ~offset_mask) == 0);
    set((get_int() & ~offset_mask) | offset);
  }
  std::uint32_t get_offset() { return ((std::uint32_t)ptr) & offset_mask; }
  static constexpr dev_ptr create(hw::dir d, uint32_t offset) {
    assert((offset & ~offset_mask) == 0);
    return dev_ptr{(decltype(dev_ptr::ptr))(get_base_addr(d) + offset)};
  }
  static constexpr dev_ptr create(hw::parity p, uint32_t offset) {
    return create(get_self_dir(p));
  }
};

/// Validate that the layout is the same on device and host
static_assert(sizeof(dev_ptr<void>) == sizeof(std::uint32_t) &&
              sizeof(dev_ptr<void>) == 4);

/// linker script details. any change here need to be reflected in the linker
/// script and vice versa.
/// This will probably be made dynamic in the future to allow more efficient
/// memory usage
/// Variable below are offsets so the need to be used in combination with base
/// tile addresses. when viewed from the host tile base address is 0 so
/// offsets can be used alone when accessing from the host.
class offset_table {
#ifndef __SYCL_DEVICE_ONLY__
  public:
#endif
/// start of the global variable section and the end of the heap.
uint32_t global_variable_start;
/// start of the heap and the end of the tile memory section.
uint32_t heap_start;

static offset_table* get(hw::dir d) {
#ifdef __SYCL_DEVICE_ONLY__
  return get_object<offset_table>(get_offset_table_begin_offset(), d);
#else
  assert(false && "unknown on the host");
  return nullptr;
#endif
}

public:
/// contain the stack
static constexpr uint32_t __attribute__((const)) get_stack_begin_offset(hw::dir d = hw::dir::self) { return 0x0; }
static constexpr uint32_t __attribute__((const)) get_stack_size(hw::dir d = hw::dir::self) { return 0x1000; }
static constexpr uint32_t __attribute__((const)) get_stack_end_offset(hw::dir d = hw::dir::self) { return get_stack_begin_offset() + get_stack_size(); }

/// contain the dynamic part of this class.
static constexpr uint32_t __attribute__((const)) get_offset_table_begin_offset(hw::dir d = hw::dir::self) { return get_stack_end_offset(); }
static constexpr uint32_t __attribute__((const)) get_offset_table_size(hw::dir d = hw::dir::self) { return sizeof(offset_table); }
static constexpr uint32_t __attribute__((const)) get_offset_table_end_offset(hw::dir d = hw::dir::self) { return get_offset_table_begin_offset() + get_offset_table_size(); }

/// contains the RPC system.
static constexpr uint32_t __attribute__((const)) get_rpc_record_begin_offset(hw::dir d = hw::dir::self) { return get_offset_table_end_offset(); }
static constexpr uint32_t __attribute__((const)) get_rpc_record_size(hw::dir d = hw::dir::self) { return 56; }
static constexpr uint32_t __attribute__((const)) get_rpc_record_end_offset(hw::dir d = hw::dir::self) { return get_rpc_record_begin_offset() + get_rpc_record_size(); }

/// Beyond this point many offsets and size are not constexpr because they
/// depend on what kernel was loaded on the tile.

/// contains the memory modules that are shared across tiles.
/// Technically all section are shared but sections other then this one should
/// not be access by your neighbors.
static constexpr uint32_t __attribute__((const)) get_tile_mem_begin_offset(hw::dir d = hw::dir::self) { return get_rpc_record_end_offset(); }
static uint32_t __attribute__((const)) get_tile_mem_size(hw::dir d = hw::dir::self) { return get_tile_mem_end_offset() - get_tile_mem_begin_offset(); }
static uint32_t __attribute__((const)) get_tile_mem_end_offset(hw::dir d = hw::dir::self) { return get_heap_begin_offset(); }

// This is the heap that is used by dynamic allocations and the allocator's bookeeping.
static uint32_t __attribute__((const)) get_heap_begin_offset(hw::dir d = hw::dir::self) { return get(d)->heap_start; }
static uint32_t __attribute__((const)) get_heap_size(hw::dir d = hw::dir::self) { return get_heap_end_offset() - get_heap_begin_offset(); }
static uint32_t __attribute__((const)) get_heap_end_offset(hw::dir d = hw::dir::self) { return get_global_variable_start(); }

static uint32_t __attribute__((const)) get_global_variable_start(hw::dir d = hw::dir::self) { return get(d)->global_variable_start; }
static uint32_t __attribute__((const)) get_global_variable_size(hw::dir d = hw::dir::self) { return get_global_variable_end_offset() - get_global_variable_end_offset(); }
static constexpr uint32_t __attribute__((const)) get_global_variable_end_offset(hw::dir d = hw::dir::self) { return tile_size; }

};

static_assert(sizeof(offset_table) == 8, "sizeof 1 we beak alignment");

} // namespace hw

} // namespace trisycl::vendor::xilinx::acap

#endif
