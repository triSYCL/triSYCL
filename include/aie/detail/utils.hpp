
#ifndef AIE_DETAIL_UTILS_HPP
#define AIE_DETAIL_UTILS_HPP

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <type_traits>

namespace aie::detail {

template<auto val>
struct wrapper {};

template <bool b, typename...> struct compile_fail_impl {
  static_assert(b, "voluntary compile error");
};

template <typename... Tys>
struct emit_diag_with : compile_fail_impl<false, Tys...> {};

template <auto... vals>
struct emit_diag_with_val : compile_fail_impl<false, wrapper<vals>...> {};

/// using this is better than using static_assert because the values of template
/// parameters will be printed in the template instantiation stack
template <auto V1, auto V2> struct assert_equal {
  static_assert(V1 == V2, "");
};

template <typename DeviceTy, typename HostTy,
          auto expected_size = sizeof(DeviceTy),
          auto expected_align = alignof(DeviceTy)>
struct assert_equal_layout {
  /// The device layout is important on device and host.
  assert_equal<sizeof(DeviceTy), expected_size> device_sizeof_check;
  assert_equal<alignof(DeviceTy), expected_align> device_alignof_check;
  /// The layout on the host in only important on the host.
#ifndef __SYCL_DEVICE_ONLY__
  assert_equal<sizeof(HostTy), expected_size> host_sizeof_check;
  assert_equal<alignof(HostTy), expected_align> host_alignof_check;
#endif
};

class empty {};

template <std::size_t Size> class non_zero_padding {
  char padding[Size] = {};
};

/// This should be used as a base class or it wont be able to have 0 padding
/// added.
template <std::size_t Size>
class padding
    : std::conditional<Size == 0, empty, non_zero_padding<Size>>::type {};

/// Provides storage suitable for a type T.
template <typename T> struct storage {
  alignas(T) char data[sizeof(T)];
  T& get() { return *reinterpret_cast<T*>(&data[0]); }
};

template <typename T> struct singleton {
  static T* instance() {
    static T elem;
    return &elem;
  }
};

auto constexpr underlying_value = [](auto e) {
  using value_t = std::underlying_type_t<decltype(e)>;
  return static_cast<value_t>(e);
};

/// check that BinStart start by an ELF magic
bool isELFMagic(const char* BinStart) {
  return BinStart[0] == 0x7f && BinStart[1] == 'E' && BinStart[2] == 'L' &&
         BinStart[3] == 'F';
}

/// Check that val has a single bit that is set.
template <typename T> constexpr bool is_single_bit(T val) {
  return (val != 0) && ((val & (val >> 1)) == 0);
}

/// align val down to align
template <typename T> constexpr T align_down(T val, T align) {
  assert(is_single_bit(align));
  return val & ~(align - 1);
}

/// align val up to align
template <typename T> constexpr T align_up(T val, T align) {
  assert(is_single_bit(align));
  return align_down(val + align - 1, align);
}

/// with no copy constructor, inherit from it to remove the copy constructor
struct move_only {
  move_only() = default;
  move_only(const move_only&) = delete;
  move_only(move_only&&) = default;
  move_only& operator=(const move_only&) = delete;
  move_only& operator=(move_only&&) = delete;
};

/// with no copy or move constructor, inherit from it to remove the copy and
/// move constructor
struct no_move {
  no_move() = default;
  no_move(const no_move&) = delete;
  no_move& operator=(const no_move&) = delete;
};

/// Allow to cast pointer type while keeping them volatile.
/// See static_asserts below for examples.
template <typename T, typename PtrT>
using keep_volatile_ptr =
    std::conditional_t<std::is_volatile_v<std::remove_pointer_t<PtrT>>,
                       volatile T*, T*>;

static_assert(std::is_same_v<keep_volatile_ptr<int, volatile char*>, volatile int*>, "");
static_assert(std::is_same_v<keep_volatile_ptr<int, char*>, int*>, "");

template <typename SrcT, typename DestT>
void maybe_volatile_memcpy(DestT* dst, SrcT* src, std::size_t size) {
  int idx = 0;
  /// Using as many 4-byte accesses as possible before falling back to 1-byte
  /// access make the IR easier to read and the generated code better.
  for (;idx + 3 < size; idx++)
    ((keep_volatile_ptr<uint32_t, DestT*>)dst)[idx] = ((keep_volatile_ptr<uint32_t, SrcT*>)src)[idx];
  for (;idx < size; idx += 1)
    dst[idx] = src[idx];
}

template <typename T>
__attribute__((noinline)) T volatile_load(volatile T* ptr) {
  T ret;
  maybe_volatile_memcpy((char*)&ret, (volatile char*)ptr, sizeof(T));
  return ret;
}

template <typename T>
__attribute__((noinline)) void volatile_store(volatile T* ptr, T t) {
  maybe_volatile_memcpy((volatile char*)ptr, (char*)&t, sizeof(T));
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

static void write_number(auto write, int i,
                         const char* base_char = "0123456789") {
  if (i < 0)
    write('-');
  /// For 0 print 0 instead of nothing.
  if (i == 0) {
    write(base_char[0]);
    return;
  }

  int base = strlen(base_char);
  int digit_count = 1;
  int tmp = i;
  while (tmp >= base || tmp <= -base) {
    digit_count++;
    tmp = tmp / base;
  }
  for (int d = digit_count; d > 0; d--)
    write(base_char[(std::abs(i / pow(base, d - 1)) % base)]);
}

}

#endif
