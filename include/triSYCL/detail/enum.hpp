#ifndef TRISYCL_SYCL_DETAIL_ENUM_HPP
#define TRISYCL_SYCL_DETAIL_ENUM_HPP

/** \file

    Some enum-related helpers

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

namespace trisycl::detail {

/** Get the enum value as an integral value of the underlying integer
    type
*/
auto constexpr underlying_value = [] (auto e) {
  using value_t = std::underlying_type_t<decltype(e)>;
  return static_cast<value_t>(e);
};


/// Get the distance between 2 enum underlying values
auto constexpr enum_distance = [] (auto enum_start, auto enum_end) {
  static_assert(std::is_same_v<decltype(enum_start), decltype(enum_end)>,
                "enum_start and auto enum_end should have the same type");
  return underlying_value(enum_end) - underlying_value(enum_start);
};


}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_ENUM_HPP
