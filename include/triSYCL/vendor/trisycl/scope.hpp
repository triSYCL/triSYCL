#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_HPP

/** \file An extension adding conceptual scope storage to SYCL objects

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/vendor/trisycl/scope/device.hpp"
#include "triSYCL/vendor/trisycl/scope/platform.hpp"
#include "triSYCL/vendor/trisycl/scope/queue.hpp"

/// This is an extension providing a conceptual API for devices, platforms...
#define SYCL_VENDOR_TRISYCL_CONCEPTUAL_API 1

/// This is an extension providing scope to devices & platforms
#define SYCL_VENDOR_TRISYCL_SCOPE 1

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_HPP
