#ifndef TRISYCL_SYCL_DETAIL_GLOBAL_CONFIG_HPP
#define TRISYCL_SYCL_DETAIL_GLOBAL_CONFIG_HPP

/** \file The OpenCL SYCL details on the global triSYCL configuration

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

/** \addtogroup defaults Manage default configuration and types
    @{
*/

// The following symbols can be set to implement a different version
#ifndef CL_SYCL_LANGUAGE_VERSION
/// This implements SYCL 1.2.1
#define CL_SYCL_LANGUAGE_VERSION 121
#endif

#ifndef TRISYCL_CL_LANGUAGE_VERSION
/// This implements triSYCL 1.2.1
#define TRISYCL_CL_LANGUAGE_VERSION 121
#endif

#define BOOST_COMPUTE_CL_VERSION_1_2

/// This source is compiled by a single source compiler
#define __SYCL_SINGLE_SOURCE__


/* Work-around an old Boost.CircularBuffer bug if a pre 1.62 Boost
   version is used */
#define TRISYCL_MAKE_BOOST_CIRCULARBUFFER_THREAD_SAFE


/** Define TRISYCL_OPENCL to add OpenCL

    triSYCL can indeed work without OpenCL if only host support is needed.
*/
#ifdef TRISYCL_OPENCL

// SYCL interoperation API with OpenCL requires some OpenCL C types:
#if defined(__APPLE__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
// But the triSYCL OpenCL implementation is actually based on Boost.Compute
#include <boost/compute.hpp>
/// A macro to keep some stuff in OpenCL mode
// Use variadic macros so we can have stray commas in the argument
#define TRISYCL_OPENCL_ONLY(...) __VA_ARGS__
#else
/// A macro to skip stuff when not supporting OpenCL
#define TRISYCL_OPENCL_ONLY(...)
#endif

#ifdef TRISYCL_DEVICE
/// Define the SYCL marker according to specification
#define __SYCL_DEVICE_ONLY__
// To keep something when in device mode
// Use variadic macros so we can have stray commas in the argument
#define TRISYCL_DEVICE_ONLY(...) __VA_ARGS__
#else
// Do not keep when not in device mode
#define TRISYCL_DEVICE_ONLY(...)
#endif

#ifdef TRISYCL_DEBUG
// To keep something when in debug mode only
// Use variadic macros so we can have stray commas in the argument
#define TRISYCL_DEBUG_ONLY(...) __VA_ARGS__
#else
// Do not keep when not in debug mode
#define TRISYCL_DEBUG_ONLY(...)
#endif


/// @} End the defaults Doxygen group

// Compiler specific weak linking (until changing to C++17 inline variables/functions)
#ifndef TRISYCL_WEAK_ATTRIB_PREFIX
#ifdef _MSC_VER
#define TRISYCL_WEAK_ATTRIB_PREFIX __declspec(selectany)
#define TRISYCL_WEAK_ATTRIB_SUFFIX
#else
#define TRISYCL_WEAK_ATTRIB_PREFIX
#define TRISYCL_WEAK_ATTRIB_SUFFIX __attribute__((weak))
#endif
#endif

// Suppress usage/leak of macros originating from Visual C++ headers
#ifdef _MSC_VER
#define NOMINMAX
#endif

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_GLOBAL_CONFIG_HPP
