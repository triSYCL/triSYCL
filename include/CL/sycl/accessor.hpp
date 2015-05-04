#ifndef TRISYCL_SYCL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_HPP

/** \file The OpenCL SYCL accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/accessor/detail/accessor.hpp"

namespace cl {
namespace sycl {

template <typename T, std::size_t dimensions> struct buffer;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename DataType,
          std::size_t Dimensions,
          access::mode Mode,
          access::target Target = access::global_buffer>
struct accessor : detail::accessor<DataType, Dimensions, Mode, Target> {
  /// \todo in the specification: store the dimension for user request
  static constexpr auto dimensionality = Dimensions;
  /// \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = DataType;
  using value_type = DataType;

  // Inherit of the constructors to have accessor constructor from detail
  using detail::accessor<DataType, Dimensions, Mode, Target>::accessor;

  /** Create an accessor to the given buffer

      \todo fix the specification to rename target that shadows template
      parameter
  */
  accessor(buffer<DataType, Dimensions> &target_buffer) :
    detail::accessor<DataType, Dimensions, Mode, Target> { *target_buffer.implementation } {}

};

/// @} End the data Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_HPP
