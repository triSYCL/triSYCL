#ifndef TRISYCL_SYCL_DETAIL_GLOBAL_CONFIG_HPP
#define TRISYCL_SYCL_DETAIL_GLOBAL_CONFIG_HPP

/** \file The OpenCL SYCL details on the global triSYCL configuration

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

/// This implement SYCL 1.2
#define CL_SYCL_LANGUAGE_VERSION 120

/// This implement triSYCL 1.2
#define CL_TRISYCL_LANGUAGE_VERSION 120

/// This source is compiled by a single source compiler
#define __SYCL_SINGLE_SOURCE__


/** Define TRISYCL_OPENCL to add OpenCL

    triSYCL can indeed work without OpenCL if only host support is needed.

    Right now it is set by Doxygen to generate the documentation.

    \todo Use a macro to check instead if the OpenCL header has been
    included before.

    But what is the right one? __OPENCL_CL_H? __OPENCL_C_VERSION__? CL_HPP_?
    Mostly CL_HPP_ to be able to use param_traits<> from cl.hpp...
*/
#ifdef TRISYCL_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#endif

/** Allow the asynchronous implementation of tasks */
#ifndef TRISYCL_ASYNC
/** Use asynchronous tasks by default.

    Is set to 0, the functors are executed synchronously.
 */
#define TRISYCL_ASYNC 0
#endif

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_GLOBAL_CONFIG_HPP
