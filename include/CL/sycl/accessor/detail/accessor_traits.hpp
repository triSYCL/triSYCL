#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_TRAITS_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_TRAITS_HPP

/** \file Some information common to SYCL accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/container_element_aspect.hpp"

namespace cl::sycl::detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

template <typename T,
          int Dimensions,
          access::mode Mode,
          access::target Target>
class accessor_traits : public detail::container_element_aspect<T> {

public:

  /** \todo in the specification: store the dimension for user request

      \todo Use another name, such as from C++2a committee discussions.
   */
  static constexpr auto dimensionality = Dimensions;

  //* \todo in the specification: store the mode for user request
  static constexpr auto mode = Mode;

  //* \todo in the specification: store the target for user request
  static constexpr auto target = Target;

};

/// @} End the data Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_TRAITS_HPP
