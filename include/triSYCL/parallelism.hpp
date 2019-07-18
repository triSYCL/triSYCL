#ifndef TRISYCL_SYCL_PARALLELISM_HPP
#define TRISYCL_SYCL_PARALLELISM_HPP

/** \file

    Implement parallel constructions to launch kernels

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_TBB
#include "triSYCL/parallelism/detail/parallelism_tbb.hpp"
#else
#include "triSYCL/parallelism/detail/parallelism.hpp"
#endif

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PARALLELISM_HPP
