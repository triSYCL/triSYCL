#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_HPP

/** \file The OpenCL triSYCL host platform implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include <memory>

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/detail/singleton.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/info/platform.hpp"
#include "triSYCL/platform/detail/platform.hpp"

namespace trisycl {

namespace extension::ce {
  class platform;
}

namespace detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// SYCL host platform
class host_platform : public detail::platform,
                      public detail::singleton<host_platform> {

friend extension::ce::platform;

// \todo Have this compatible with has_extension
vector_class<string_class> platform_extensions { "Xilinx_blocking_pipes" };

public:

#ifdef TRISYCL_OPENCL
  /** Return the cl_platform_id of the underlying OpenCL platform

      This throws an error since there is no OpenCL platform associated
      to the host platform.
  */
  cl_platform_id get() const override {
    throw non_cl_error("The host platform has no OpenCL platform");
  }


  /** Return the underlying Boost.Compute platform

      This throws an error since there is no Boost Compute platform associated
      to the host platform.
   */
  boost::compute::platform &get_boost_compute() const override {
    throw
      non_cl_error("The host device has no underlying Boost Compute platform");
  }
#endif


  /// Return true since this platform is the SYCL host platform
  bool is_host() const override {
    return true;
  }


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

    default:
      // \todo Define some SYCL exception type for this type of errors
      throw std::invalid_argument {
        "Unknown parameter value for SYCL platform information" };
    }
  }

  vector_class<string_class> get_extension_strings(void) const override {
    return platform_extensions;
  }


  /** Specify whether a specific extension is supported on the platform

      \todo To be implemented
  */
  bool has_extension(const string_class &extension) const override {
    return std::find(platform_extensions.begin(), platform_extensions.end(), extension) !=
           platform_extensions.end();
  }

  /** Get all the available devices for the host platform

      \param[in] device_type is the device type to filter the selection
      or \c info::device_type::all by default to return all the
      devices

      \return the device list
  */
  vector_class<trisycl::device>
  get_devices(const device_selector &device_selector) const override;

};

/// @} to end the execution Doxygen group

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
