#ifndef TRISYCL_SYCL_DETAIL_LINEAR_ID_HPP
#define TRISYCL_SYCL_DETAIL_LINEAR_ID_HPP

/** \file Compute linearized array access

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

namespace trisycl::detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/

/** Compute a linearized array access used in the OpenCL 2 world

    Typically for the get_global_linear_id() and get_local_linear_id()
    functions.
*/
template <typename Range, typename Id>
size_t constexpr inline linear_id(Range range, Id id, Id offset = {}) {
  auto dims = std::distance(std::begin(range), std::end(range));

  size_t linear_id = 0;
  /* A good compiler should unroll this and do partial evaluation to
     remove the first multiplication by 0 of this Horner evaluation and
     remove the 0 offset evaluation */
  for (int i = dims - 1; i >= 0; --i)
    linear_id = linear_id*range[i] + id[i] - offset[i];

  return linear_id;
}


/// @} End the helpers Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_LINEAR_ID_HPP
