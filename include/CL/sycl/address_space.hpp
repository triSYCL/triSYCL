#ifndef TRISYCL_SYCL_ADDRESS_SPACE_HPP
#define TRISYCL_SYCL_ADDRESS_SPACE_HPP

/** \file

    Implement OpenCL address spaces in SYCL with C++-style.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {

/** \addtogroup address_spaces Dealing with OpenCL address spaces
    @{
*/

/** Enumerate the different OpenCL 2 address spaces */
enum address_space {
  constant_address_space,
  generic_address_space,
  global_address_space,
  local_address_space,
  private_address_space,
};

}
}
/// @} End the address_spaces Doxygen group


#include "CL/sycl/address_space/detail/address_space.hpp"


namespace cl {
namespace sycl {

/** \addtogroup address_spaces
    @{
*/

/** Declare a variable to be an OpenCL constant pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T>
using constant = detail::addr_space<T, constant_address_space>;


/** Declare a variable to be an OpenCL 2 generic pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T>
using generic = detail::addr_space<T, generic_address_space>;


/** Declare a variable to be an OpenCL global pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T>
using global = detail::addr_space<T, global_address_space>;


/** Declare a variable to be an OpenCL local pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T>
using local = detail::addr_space<T, local_address_space>;


/** Declare a variable to be an OpenCL private pointer

    \param T is the pointer type

    Note that if \a T is not a pointer type, it is an error.
*/
template <typename T>
using priv = detail::addr_space<T, private_address_space>;


/** A pointer that can be statically associated to any address-space

    \param Pointer is the pointer type

    \param AS is the address space to point to

    Note that if \a Pointer is not a pointer type, it is an error.
*/
template <typename Pointer, address_space AS>
using multi_ptr = detail::address_space_ptr<Pointer, AS>;


/** Construct a cl::sycl::multi_ptr<> with the right type

    \param pointer is the address with its address space to point to

    \todo Implement the case with a plain pointer
*/
template <typename T, address_space AS>
multi_ptr<T, AS> make_multi(multi_ptr<T, AS> pointer) {
  return pointer;
}

}
}
/// @} End the parallelism Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ADDRESS_SPACE_HPP
