#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_PLATFORM_HPP

/** \file The OpenCL triSYCL platform information

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include "CL/sycl/info/platform.hpp"
#include "CL/sycl/detail/global_config.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** A metafunction returning the parameters for the host platform
    implementation */
template <info::platform Param>
inline string_class get_host_platform_info() {
  // \todo Define some SYCL exception type for this type of errors
  throw std::invalid_argument {
    "Unknown parameter valuefor SYCL platform information" };
}

/// The metafunction value for the profile
template <>
inline string_class get_host_platform_info<info::platform::profile>() {
  /* Well... Is the host platform really a full profile whereas it is not
     really OpenCL? */
  return "FULL_PROFILE";
}

/// The metafunction value for the version
template <>
inline string_class get_host_platform_info<info::platform::version>() {
  // \todo I guess it should include the software version too...
  return "1.2";
}

/// The metafunction value for the name
template <>
inline string_class get_host_platform_info<info::platform::name>() {
  return "triSYCL host platform";
}

/// The metafunction value for the vendor
template <>
inline string_class get_host_platform_info<info::platform::vendor>() {
  return "triSYCL Open Source project";
}

/// The metafunction value for the extensions
template <>
inline string_class get_host_platform_info<info::platform::extensions>() {
  // No extension
  return "";
}

/// @} to end the execution Doxygen group

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

#endif // TRISYCL_SYCL_PLATFORM_DETAIL_PLATFORM_HPP
