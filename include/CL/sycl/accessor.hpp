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

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename dataType,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct accessor : detail::AccessorImpl<dataType, dimensions, mode, target> {
  /// \todo in the specification: store the dimension for user request
  static const auto dimensionality = dimensions;
  /// \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = dataType;
  using value_type = dataType;

  // Use a short-cut to the implementation because type name becomes quite
  // long...
  using Impl = detail::AccessorImpl<dataType, dimensions, mode, target>;

  // Inherit of the constructors to have accessor constructor from BufferImpl
  using Impl::AccessorImpl;

  /// Create an accessor to the given buffer
  // \todo fix the specification to rename target that shadows template parm
  accessor(buffer<dataType, dimensions> &targetBuffer) :
    Impl(*targetBuffer.Impl) {}

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
