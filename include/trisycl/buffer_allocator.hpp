#ifndef TRISYCL_SYCL_BUFFER_ALLOCATOR_HPP
#define TRISYCL_SYCL_BUFFER_ALLOCATOR_HPP

/** \file The OpenCL SYCL buffer_allocator

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

namespace trisycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The default buffer allocator used by the runtime, when no allocator is
    defined by the user

    Reuse the C++ default allocator.
*/
template <typename T>
using buffer_allocator = std::allocator<T>;

/// @} End the data Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_BUFFER_ALLOCATOR_HPP
