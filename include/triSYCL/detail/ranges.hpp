#ifndef TRISYCL_SYCL_DETAIL_RANGES_HPP
#define TRISYCL_SYCL_DETAIL_RANGES_HPP

/** \file

    Some range-related helpers

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <range/v3/all.hpp>

namespace trisycl::views {

/** Construct a range of integral values iterating between 2 enum
    values, inclusive

    \param begin is the starting value

    \param inclusive_end is the value to stop at.

    Note that since \p inclusive_end is included in the range, the
    range has at least 1 element.

    \return a range of integral values of the underlying type of the
    enum from \p begin to the \p inclusive_end
*/
auto enum_underlying_type = [] (auto begin, auto inclusive_end) {
  using begin_t = decltype(begin);
  using inclusive_end_t = decltype(inclusive_end);

  static_assert(std::is_same_v<begin_t, inclusive_end_t>,
                "begin and inclusive_end should be from the same enum");
  using value_t = std::underlying_type_t<begin_t>;
  return ranges::iota_view { static_cast<value_t>(begin),
                             static_cast<value_t>(inclusive_end) + 1 };
};


/** Construct a range iterating between 2 enum values, inclusive

    \param begin is the starting value

    \param inclusive_end is the value to stop at.

    Note that since \p inclusive_end is included in the range, the
    range has at least 1 element.

    \return a range of enum values from \p begin to the \p
    inclusive_end
*/
auto enum_type = [] (auto begin, auto inclusive_end) {
  using begin_t = decltype(begin);

  return enum_underlying_type(begin, inclusive_end)
    | ranges::views::transform([] (auto i) { return static_cast<begin_t>(i); });
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_RANGES_HPP
