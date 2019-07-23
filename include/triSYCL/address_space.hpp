#ifndef TRISYCL_SYCL_ADDRESS_SPACE_HPP
#define TRISYCL_SYCL_ADDRESS_SPACE_HPP

/** \file

    Implement OpenCL address spaces in SYCL with C++-style.

    Note that in SYCL 1.2, only pointer types should be specified but
    in this implementation we generalize the concept to any type.

    \todo Add the alias ..._ptr<T> = ...<T *>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl {

/** \addtogroup address_spaces Dealing with OpenCL address spaces
    @{
*/

namespace access {

/** Enumerate the different OpenCL 2 address spaces */
enum class address_space {
  global_space,
  local_space,
  constant_space,
  private_space,
  generic_space,
};

}
/// @} End the address_spaces Doxygen group
}

#include "triSYCL/address_space/detail/address_space.hpp"


namespace trisycl {

/** \addtogroup address_spaces
    @{
*/

/** Declare a variable to be in the OpenCL constant address space

    \param T is the type of the object
*/
template <typename T>
using constant = detail::addr_space<T, access::address_space::constant_space>;


/** Declare a variable to be in the OpenCL constant address space

    \param T is the type of the object
*/
template <typename T>
using constant_ptr = constant<T*>;


/** Declare a variable to be in the OpenCL 2 generic address space

    \param T is the type of the object
*/
template <typename T>
using generic = detail::addr_space<T, access::address_space::generic_space>;


/** Declare a variable to be in the OpenCL global address space

    \param T is the type of the object
*/
template <typename T>
using global = detail::addr_space<T, access::address_space::global_space>;


/** Declare a variable to be in the OpenCL global address space

    \param T is the type of the object
*/

template <typename T>
using global_ptr = global<T*>;


/** Declare a variable to be in the OpenCL local address space

    \param T is the type of the object
*/
template <typename T>
using local = detail::addr_space<T, access::address_space::local_space>;


/** Declare a variable to be in the OpenCL local address space

    \param T is the type of the object
*/
template <typename T>
using local_ptr = local<T*>;


/** Declare a variable to be in the OpenCL private address space

    \param T is the type of the object
*/
template <typename T>
using priv = detail::addr_space<T, access::address_space::private_space>;


/** Declare a variable to be in the OpenCL private address space

    \param T is the type of the object
*/
template <typename T>
using private_ptr = priv<T*>;


/** A pointer that can be statically associated to any address-space

    \param Pointer is the pointer type

    \param AS is the address space to point to

    Note that if \a Pointer is not a pointer type, it is an error.
*/
template <typename Pointer, access::address_space AS>
using multi_ptr = detail::address_space_ptr<Pointer, AS>;


/** Construct a trisycl::multi_ptr<> with the right type

    \param pointer is the address with its address space to point to

*/
template <typename T, access::address_space AS>
multi_ptr<T, AS> make_ptr(multi_ptr<T, AS> pointer) {
  return pointer;
}

/** Construct a trisycl::multi_ptr<> with the right type

    \todo Implement the case with a plain pointer
*/
template <typename T, access::address_space AS>
multi_ptr<T, AS> make_ptr(T *pointer) {
  TRISYCL_UNIMPL;
  return pointer;
}
}
/// @} End the address_spaces Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ADDRESS_SPACE_HPP
