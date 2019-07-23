#ifndef TRISYCL_SYCL_DETAIL_SPIR_OPENCL_SPIR_HELPERS_HPP
#define TRISYCL_SYCL_DETAIL_SPIR_OPENCL_SPIR_HELPERS_HPP

/** \file
    Adds helper functions to populate Id's and Range's with OpenCL SPIR
    intrinsics data: e.g. get_global_id(0)

    \todo implement group and nd_item and to add nd_item to
      create_parallel_for_arg

    andrew point gozillon at yahoo point com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include <type_traits>

#include "triSYCL/detail/SPIR/opencl_spir_req.h"
#include "triSYCL/group.hpp"
#include "triSYCL/h_item.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"
#include "triSYCL/nd_range.hpp"
#include "triSYCL/range.hpp"

/** \addtogroup opencl_spir_helpers
    @{
*/

namespace trisycl::detail::spir {

// Fills the small_array_123 of the type passed to it up to the dimensionality
// of the type T (T generally intended to be a sycl id or range, but can be
// anything with a dimensionality variable defined and a subscript operator)
template <typename T,  typename F>
auto make_array_with_func(F unary_func) {
  T id_or_range {};

  for (size_t i = 0; i < id_or_range.dimensionality; ++i)
    id_or_range[i] = unary_func(i);

  return id_or_range;
}

template <int Dimensions>
nd_range<Dimensions> make_spir_nd_range() {
  return nd_range<Dimensions> {
    make_array_with_func<range<Dimensions>>(get_global_size),
    make_array_with_func<range<Dimensions>>(get_local_size),
    make_array_with_func<id<Dimensions>>(get_global_offset)
  };
}

// template <int Dimensions>
// nd_item<Dimensions> make_spir_nd_item() {}

template <int Dimensions>
h_item<Dimensions> make_spir_h_item() {
  return h_item<Dimensions> {
    make_array_with_func<id<Dimensions>>(get_global_offset),
    make_spir_nd_range<Dimensions>()
  };
}

// group
// template <int Dimensions>
// group<Dimensions> make_spir_group() {}

// Uses a passed in index or range to identify the required index or range type
// to generate
template <int Dimensions, typename Index_T>
auto create_parallel_for_arg(Index_T index_or_range) {
  if constexpr (std::is_same_v<Index_T, id<Dimensions>>) {
    index_or_range = make_array_with_func<Index_T>(get_global_id);
  } else if constexpr (std::is_same_v<Index_T, item<Dimensions>>) {
    // Item's do not contain local information in parallel_for
    index_or_range = Index_T {
      make_array_with_func<range<Dimensions>>(get_global_size),
      make_array_with_func<id<Dimensions>>(get_global_id),
      make_array_with_func<id<Dimensions>>(get_global_offset)
    };
  } else {
    static_assert(std::is_same_v<Index_T, item<Dimensions>> ||
                  std::is_same_v<Index_T, id<Dimensions>>,
                  "undefined Index_T passed to create_parallel_for_arg");
  }

  return index_or_range;
}

/// @} End the parallelism Doxygen group

} // namespace trisycl::details::spir

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SPIR_OPENCL_SPIR_HELPERS_HPP
