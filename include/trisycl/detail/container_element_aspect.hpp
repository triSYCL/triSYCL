#ifndef TRISYCL_SYCL_DETAIL_CONTAINER_ELEMENT_ASPECT_HPP
#define TRISYCL_SYCL_DETAIL_CONTAINER_ELEMENT_ASPECT_HPP

/** \file Implement basic types à la STL related to container
    elements, such as value_type, reference...

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/

/// A mix-in to add some container element aspects
template <typename T>
struct container_element_aspect {

  using value_type = T;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;

};

/// @} End the helpers Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_CONTAINER_ELEMENT_ASPECT_HPP
