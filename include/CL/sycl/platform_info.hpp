#ifndef TRISYCL_SYCL_PLATFORM_INFO_HPP
#define TRISYCL_SYCL_PLATFORM_INFO_HPP

/** \file The OpenCL SYCL platform

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/global_config.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/
namespace info {

enum class device_type : unsigned int {
  cpu,
  gpu,
  accelerator,
  custom,
  defaults,
  host,
  all
};


/** Platform information descriptors

    A SYCL platform can be queried for all of the following information
    using the get_info function. All SYCL contexts have valid devices for
    them, including the SYCL host device.

    In this implementation, the values are mapped to OpenCL values to
    avoid further remapping later
*/
enum class platform : unsigned int {
  /** Returns the profile name (as a string_class) supported by the
      implementation.

      Can be either FULL PROFILE or EMBEDDED PROFILE.
  */
  profile TRISYCL_SKIP_OPENCL(= CL_PLATFORM_PROFILE),
  /** Returns the OpenCL software driver version string in the form major
      number.minor number (as a string_class)
  */
  version TRISYCL_SKIP_OPENCL(= CL_PLATFORM_VERSION),
  /** Returns the name of the platform (as a string_class)
  */
  name TRISYCL_SKIP_OPENCL(= CL_PLATFORM_NAME),
  /** Returns the string provided by the platform vendor (as a string_class)
  */
  vendor TRISYCL_SKIP_OPENCL(= CL_PLATFORM_VENDOR),
  /** Returns a space-separated list of extension names supported by the
      platform (as a string_class)
  */
  extensions TRISYCL_SKIP_OPENCL(= CL_PLATFORM_EXTENSIONS)
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

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PLATFORM_INFO_HPP
