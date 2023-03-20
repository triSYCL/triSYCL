
#ifndef AIE_DETAIL_UTILS_HPP
#define AIE_DETAIL_UTILS_HPP

#include <cstdint>
#include <type_traits>

namespace aie::detail {

template <bool b, typename...> struct compile_fail_impl {
  static_assert(b, "voluntary compile error");
};

template <typename... Tys>
struct emit_diag_with : compile_fail_impl<false, Tys...> {};

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
}

#endif
