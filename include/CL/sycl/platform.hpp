#ifndef TRISYCL_SYCL_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_HPP

/** \file The OpenCL SYCL platform

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/global_config.hpp"

#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device.hpp"
#include "CL/sycl/platform/detail/host_platform.hpp"
#ifdef TRISYCL_OPENCL
#include "CL/sycl/platform/detail/opencl_platform.hpp"
#endif
#include "CL/sycl/platform/detail/host_information.hpp"
#include "CL/sycl/platform/detail/platform.hpp"
#include "CL/sycl/info/platform.hpp"

namespace cl {
namespace sycl {

class device_selector;
class device;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** Abstract the OpenCL platform

    \todo triSYCL Implementation
*/
class platform
  /* Use the underlying platform implementation that can be shared in the
     SYCL model */
  : public detail::shared_ptr_implementation<platform, detail::platform> {

  // The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<platform, detail::platform>;

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;


public:

  /** Default constructor for platform which is the host platform

      Returns errors via the SYCL exception class.
  */
  platform() : implementation_t { detail::host_platform::instance() } {}


#ifdef TRISYCL_OPENCL
  /** Construct a platform class instance using cl_platform_id of the
      OpenCL device

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the OpenCL platform.
  */
  platform(cl_platform_id platform_id) :
    : device { boost::compute::platform { platform_id } } {}
#endif


  /**  Construct a platform object from the device returned by a device
       selector of the user's choice

       Returns errors via the SYCL exception class.
  */
  explicit platform(const device_selector &dev_selector) {
    detail::unimplemented();
  }


#ifdef TRISYCL_OPENCL
  /** Returns the cl_platform_id of the underlying OpenCL platform

      If the platform is not a valid OpenCL platform, for example if it is
      the SYCL host, an exception is thrown

      \todo Define a SYCL exception for this
  */
  cl_platform_id get() const {
    return implementation->get();
  }
#endif


  /// Get the list of all the platforms available to the application
  static vector_class<platform> get_platforms() {
    // Start with the default platform
    vector_class<platform> platforms { {} };
    return platforms;
  }

#if 0
  /** Returns all the available devices for this platform, of type device
      type, which is defaulted to info::device_type::all

      By default returns all the devices.

      \todo To be implemented
  */
  vector_class<device>
  get_devices(info::device_type device_type = info::device_type::all) const {
    detail::unimplemented();
    return {};
  }
#endif


  /// Get the OpenCL information about the requested parameter
  template <info::platform Param>
  typename info::param_traits<info::platform, Param>::type
  get_info() const {
#ifdef TRISYCL_OPENCL
    if (m_platform)
      /* Use the fact that the triSYCL info values are the same as the
         OpenCL ones used in Boost.Compute to just cast the enum class to
         the int value */
      return m_platform->get_info<static_cast<int>(Param)>();
#endif
    // Otherwise ask the host platform information
    return detail::get_host_platform_info<Param>();
  }


  /// Test if an extension is available on the platform
  bool has_extension(const string_class &extension) const {
    return implementation->has_extension(extension);
  }


  /// Test if this platform is a host platform
  bool is_host() const {
    return implementation->is_host();
  }

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

#endif // TRISYCL_SYCL_PLATFORM_HPP
