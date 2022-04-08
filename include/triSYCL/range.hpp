#ifndef TRISYCL_SYCL_RANGE_HPP
#define TRISYCL_SYCL_RANGE_HPP

/** \file The OpenCL SYCL range<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <functional>
#include <numeric>
#include "triSYCL/detail/small_array.hpp"

namespace trisycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A SYCL range defines a multi-dimensional index range that can be used
    to define launch parallel computation extent or buffer sizes.

    \todo use std::size_t Dimensions instead of int Dimensions in the
        specification?

    \todo add to the specification this default parameter value?

    \todo add to the specification some way to specify an offset?
*/
template <int Dimensions = 1>
class range
    : public detail::small_array_sycl<std::size_t, range<Dimensions>,
                                      Dimensions> {

 public:

  // Inherit of all the constructors
  using detail::small_array_sycl<std::size_t,
                                range<Dimensions>,
                                Dimensions>::small_array_sycl;

  range() = default;

  /// The number of dimensions of the range
  static auto constexpr rank() { return Dimensions; }

  /** Return the number of elements in the range
   */
  size_t size() const {
    // Return the product of the sizes in each dimension
    return std::accumulate(this->cbegin(),
                           this->cend(),
                           1,
                           std::multiplies<size_t> {});
  }
};

/**  User-defined deduction guides to deduce the rank of a range from
     the number of arguments

     For example range { 4, 5, 7 } will define an range<3> { 4, 5, 7 }
*/
template <typename... BasicType>
range(BasicType... Args) -> range<sizeof...(Args)>;

/// @} End the parallelism Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_RANGE_HPP
