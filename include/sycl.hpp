/** \file

    This is the SYCL extension header to expose triSYCL directly into
    the shorter ::sycl namespace instead of the official SYCL 1.2.1
    ::cl::sycl
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
