#ifndef TRISYCL_SYCL_ALLOCATOR_HPP
#define TRISYCL_SYCL_ALLOCATOR_HPP

/** \file The OpenCL SYCL allocator

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

namespace trisycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The allocator objects give the programmer some control on how the
    memory is allocated inside SYCL
*/

/** The allocator used for the \c buffer inside SYCL

    Just use the default allocator for now.
*/
template <typename T>
using buffer_allocator = std::allocator<T>;


/** The allocator used for the \c image inside SYCL

    Just use the default allocator for now.
*/
template <typename T>
using image_allocator = std::allocator<T>;


/** The allocator used to map the memory at the same place

    Just use the default allocator for now.

    \todo : implement and clarify the specification. It looks like it
    is not really an allocator according the current spec
*/
template <typename T>
using map_allocator = std::allocator<T>;


/// @} End the data Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ALLOCATOR_HPP
