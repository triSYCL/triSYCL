#ifndef TRISYCL_SYCL_DETAIL_LAYOUT_UTILS_HPP
#define TRISYCL_SYCL_DETAIL_LAYOUT_UTILS_HPP

/** \file Some utilities to deal with data layout

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

namespace trisycl::detail {

/// Using this is better than using static_assert because the values of template
/// parameters will be printed in the template instantiation stack
template<auto V1, auto V2>
struct assert_equal {
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

template <std::size_t Size> class non_zero_padding { char padding[Size] = {}; };

/// This should be used as a base class or it will not be able to have 0 padding
/// added.
template <std::size_t Size>
class padding
    : std::conditional_t<Size == 0, empty, non_zero_padding<Size>> {};

/// Provides storage suitable for a type T.
template <typename T> struct storage {
  alignas(T) char data[sizeof(T)];
  T& get() { return *reinterpret_cast<T*>(&data[0]); }
};

}

#endif
