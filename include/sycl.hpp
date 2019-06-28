#ifndef TRISYCL_SYCL_HPP
#define TRISYCL_SYCL_HPP

/** \file

    This is the SYCL extension header to expose triSYCL directly into
    the shorter \c ::sycl namespace instead of the official SYCL 1.2.1
    \c ::cl::sycl
*/


#include "trisycl.hpp"

namespace sycl = ::trisycl;

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_HPP
