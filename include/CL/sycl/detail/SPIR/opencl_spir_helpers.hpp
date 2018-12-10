#ifndef TRISYCL_SYCL_DETAIL_SPIR_OPENCL_SPIR_HELPERS_HPP
#define TRISYCL_SYCL_DETAIL_SPIR_OPENCL_SPIR_HELPERS_HPP

/** \file
    Adds helper functions to populate Id's and Range's with OpenCL SPIR
    intrinsics data: e.g. get_global_id(0)

    \todo implement group and nd_item and to add nd_item to
      create_parallel_for_arg

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include <type_traits>

#include "CL/sycl/detail/SPIR/opencl_spir_req.h"
#include "CL/sycl/group.hpp"
#include "CL/sycl/h_item.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

/** \addtogroup opencl_spir_helpers
    @{
*/

namespace cl::sycl::detail::spir {

enum class spir_gen_type {
  LOCAL=0,
  GLOBAL=1,
  OFFSET=2
};

template <int Dimensions, spir_gen_type gen_type>
id<Dimensions> gen_spir_id() {
  id<Dimensions> id;

  for (size_t i = 0; i < Dimensions; ++i)
    if constexpr (gen_type == spir_gen_type::LOCAL)
      id[i] = get_local_id(i);
    else if constexpr (gen_type == spir_gen_type::GLOBAL)
      id[i] = get_global_id(i);
    else if constexpr (gen_type == spir_gen_type::OFFSET)
      id[i] = get_global_offset(i);

  return id;
}

template <int Dimensions, spir_gen_type gen_type>
range<Dimensions> gen_spir_range() {
  static_assert(gen_type != spir_gen_type::OFFSET, "spir_gen_type offset "
    "specified for gen_spir_range, no valid spir intrinsic");

  range<Dimensions> range;

  for (size_t i = 0; i < Dimensions; ++i)
    if constexpr (gen_type == spir_gen_type::LOCAL)
      range[i] = get_local_size(i);
    else if constexpr (gen_type == spir_gen_type::GLOBAL)
      range[i] = get_global_size(i);

  return range;
}

// The Local Implementation is not required at the moment, parallel_for only
// allows items to query at a global level
template <int Dimensions, spir_gen_type gen_type>
item<Dimensions> gen_spir_item() {
  static_assert(gen_type != spir_gen_type::OFFSET, "spir_gen_type offset "
    "specified for gen_spir_item, no valid intrinsic combination");

  if constexpr (gen_type == spir_gen_type::LOCAL) {
    return item<Dimensions>{
            gen_spir_range<Dimensions, spir_gen_type::LOCAL>(),
            gen_spir_id<Dimensions,    spir_gen_type::LOCAL>(),
            gen_spir_id<Dimensions,    spir_gen_type::OFFSET>()
          };
  } else if constexpr (gen_type == spir_gen_type::GLOBAL) {
    return item<Dimensions>{
            gen_spir_range<Dimensions, spir_gen_type::GLOBAL>(),
            gen_spir_id<Dimensions,    spir_gen_type::GLOBAL>(),
            gen_spir_id<Dimensions,    spir_gen_type::OFFSET>()
          };
  }
}

template <int Dimensions>
nd_range<Dimensions> gen_spir_nd_range() {
  return nd_range<Dimensions>{
    gen_spir_range<Dimensions, spir_gen_type::GLOBAL>(),
    gen_spir_range<Dimensions, spir_gen_type::LOCAL>(),
    gen_spir_id<Dimensions, spir_gen_type::OFFSET>()
  };
}

// template <int Dimensions>
// nd_item<Dimensions> gen_spir_nd_item() {}

template <int Dimensions>
h_item<Dimensions> gen_spir_h_item() {
  return h_item<Dimensions>{
    gen_spir_id<Dimensions, spir_gen_type::GLOBAL>(),
    gen_spir_nd_range<Dimensions>()
  };
}

// group
// template <int Dimensions>
// group<Dimensions> gen_spir_group() {}

// Forwards an index type onto a generator function on the device.
template <int Dimensions, typename Index_T>
auto create_parallel_for_arg(Index_T index) {
  if constexpr (std::is_same_v<Index_T, id<Dimensions>>) {
    index = gen_spir_id<Dimensions, spir_gen_type::GLOBAL>();
  }

  if constexpr (std::is_same_v<Index_T, item<Dimensions>>) {
    index = gen_spir_item<Dimensions, spir_gen_type::GLOBAL>();
  }

  // To be implemented apon gen_spir_nd_item implementation
  // if constexpr (std::is_same_v<Index_T, nd_item<dimensions>>) {}

  return index;
}

/// @} End the parallelism Doxygen group

} // namespace cl::sycl::details::spir

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SPIR_OPENCL_SPIR_HELPERS_HPP
