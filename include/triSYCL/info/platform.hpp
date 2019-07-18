#ifndef TRISYCL_SYCL_INFO_PLATFORM_HPP
#define TRISYCL_SYCL_INFO_PLATFORM_HPP

/** \file The OpenCL SYCL platform information parameters

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/global_config.hpp"
#include "triSYCL/info/param_traits.hpp"

namespace trisycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/
namespace info {

/** Platform information descriptors

    A SYCL platform can be queried for all of the following information
    using the get_info function.

    In this implementation, the values are mapped to OpenCL values to
    avoid further remapping later when OpenCL is used
*/
enum class platform : unsigned int {
  /** Returns the profile name (as a string_class) supported by the
      implementation.

      Can be either FULL PROFILE or EMBEDDED PROFILE.
  */
  profile TRISYCL_OPENCL_ONLY(= CL_PLATFORM_PROFILE),

  /** Returns the OpenCL software driver version string in the form major
      number.minor number (as a string_class)
  */
  version TRISYCL_OPENCL_ONLY(= CL_PLATFORM_VERSION),

  /** Returns the name of the platform (as a string_class)
  */
  name TRISYCL_OPENCL_ONLY(= CL_PLATFORM_NAME),

  /** Returns the string provided by the platform vendor (as a string_class)
  */
  vendor TRISYCL_OPENCL_ONLY(= CL_PLATFORM_VENDOR),

  /** Returns a space-separated list of extension names supported by the
      platform (as a string_class)
  */
  extensions TRISYCL_OPENCL_ONLY(= CL_PLATFORM_EXTENSIONS),

#if CL_SYCL_LANGUAGE_VERSION >= 220 && defined(CL_VERSION_2_1)
  /** Returns the resolution of the host timer in nanoseconds as used by
      clGetDeviceAndHostTimer
  */
  host_timer_resolution
    TRISYCL_OPENCL_ONLY(= CL_PLATFORM_HOST_TIMER_RESOLUTION)
#endif
};


/** Query the return type for get_info() on platform parameter type

    This defines the meta-function
    \code
    param_traits<info::platform x, string_class>::type == string_class
    \endcode

    for all x, which means that get_info() returns always a string_class
    when asked about platform info.
*/
TRISYCL_INFO_PARAM_TRAITS_ANY_T(info::platform, string_class)
TRISYCL_INFO_PARAM_TRAITS(info::platform::extensions, vector_class<string_class>)
#if CL_SYCL_LANGUAGE_VERSION >= 220 && defined(CL_VERSION_2_1)
///  get_info<host_timer_resolution>() return a cl_ulong
#ifdef TRISYCL_OPENCL
TRISYCL_INFO_PARAM_TRAITS(info::platform::host_timer_resolution, cl_ulong)
#else
TRISYCL_INFO_PARAM_TRAITS(info::platform::host_timer_resolution,
                          unsigned long int)
#endif
#endif
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_INFO_PLATFORM_HPP
