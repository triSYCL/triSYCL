#ifndef TRISYCL_SYCL_SYCL_HPP
#define TRISYCL_SYCL_SYCL_HPP

/** \file

    This is the SYCL extension header to expose triSYCL directly into
    the shorter \c ::sycl namespace instead of the official SYCL 1.2.1
    \c ::cl::sycl
*/


#include "triSYCL/sycl.hpp"

/// Expose the SYCL API directly in the ::sycl namespace
namespace sycl {
  // Make the triSYCL implementation appear here
  using namespace ::trisycl;
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_SYCL_HPP
