#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_HPP

/** \file The OpenCL triSYCL host platform implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include <memory>

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/detail/singleton.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/info/platform.hpp"
#include "CL/sycl/platform/detail/platform.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// SYCL host platform
class host_platform : public detail::platform,
                      public detail::singleton<host_platform> {

// \todo Have this compatible with has_extension
auto static constexpr platform_extensions = "Xilinx_blocking_pipes";

public:

#ifdef TRISYCL_OPENCL
  /** Return the cl_platform_id of the underlying OpenCL platform

      This throws an error since there is no OpenCL platform associated
      to the host platform.
  */
  cl_platform_id get() const override {
    throw non_cl_error("The host platform has no OpenCL platform");
  }
#endif


  /// Return true since this platform is the SYCL host platform
  bool is_host() const override {
    return true;
  }


#if 0
  /** Returns at most the host device for this platform, according to
      the requested kind

      By default returns all the devices, which is obviously the host
      one here

      \todo To be implemented
  */
  vector_class<device>
  get_devices(info::device_type device_type = info::device_type::all)
    const override
  {
    detail::unimplemented();
    return {};
  }
#endif


  /** Returning the information parameters for the host platform
      implementation
  */
  string_class get_info_string(info::platform param) const override {
    switch (param) {
    case info::platform::profile:
      /* Well... Is the host platform really a full profile whereas it
         is not really OpenCL? */
      return "FULL_PROFILE";

    case info::platform::version:
      // \todo I guess it should include the software version too...
      return "2.2";

    case info::platform::name:
      return "triSYCL host platform";

    case info::platform::vendor:
      return "triSYCL Open Source project";

    case info::platform::extensions:
      return platform_extensions;

    default:
      // \todo Define some SYCL exception type for this type of errors
      throw std::invalid_argument {
        "Unknown parameter value for SYCL platform information" };
    }
  }


  /** Specify whether a specific extension is supported on the platform

      \todo To be implemented
  */
  bool has_extension(const string_class& /*extension*/) const override {
    detail::unimplemented();
    return {};
  }

};

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

#endif // TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_HPP
