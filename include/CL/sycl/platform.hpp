#ifndef TRISYCL_SYCL_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_HPP

/** \file The OpenCL SYCL platform

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/context.hpp"
#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device.hpp"

namespace cl {
namespace sycl {

class device_selector;
class device;

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
*/
enum class platform : unsigned int {
  /** Returns the profile name (as a string_class) supported by the im-
      plementation.

      Can be either FULL PROFILE or EMBEDDED PROFILE.
  */
  profile,
  /** Returns the OpenCL software driver version string in the form major
      number.minor number (as a string_class)
  */
  version,
  /** Returns the name of the platform (as a string_class)
  */
  name,
  /** Returns the string provided by the platform vendor (as a string_class)
  */
  vendor,
  /** Returns a space-separated list of extension names supported by the
      platform (as a string_class)
  */
  extensions
};


/** Query the return type for get_info() on platform stuff

    Only return a string_class
*/
TRISYCL_INFO_PARAM_TRAITS_ANY_T(info::platform, string_class)

}

/** Abstract the OpenCL platform

    \todo triSYCL Implementation
*/
class platform {


public:

#ifdef TRISYCL_OPENCL
  /** Construct a default platform and provide an optional error_handler
      to deals with errors

      \todo Add copy/move constructor to the implementation

      \todo Add const to the specification
  */
  explicit platform(cl_platform_id platformID) {
    detail::unimplemented();
  };
#endif

  /** Default constructor for platform

      It constructs a platform object to encapsulate the device returned
      by the default device selector.

      Returns errors via the SYCL exception class.

      Get back the default constructors, for this implementation.
  */
  platform() = default;


#ifdef TRISYCL_OPENCL
  /** Returns the cl_platform_id of the underlying
      OpenCL platform

      If the platform is not a valid OpenCL platform, for example if it is
      the SYCL host, a nullptr will be returned.

      \todo To be implemented
  */
  cl_platform_id get() const {
    detail::unimplemented();
    return {}; }
#endif


  /** Get the list of all the platforms available to the application

      \todo To be implemented
  */
  static vector_class<platform> get_platforms() {
    detail::unimplemented();
    return {};
  }


  /** Returns all the available devices for this platform, of type device
      type, which is defaulted to info::device_type::all


      By default returns all the devices.
  */
  vector_class<device>
  get_devices(info::device_type device_type = info::device_type::all) const {
    detail::unimplemented();
    return {};
  }


  /** Get the OpenCL information about the requested parameter

      \todo To be implemented
  */
  template <info::platform Param>
  typename info::param_traits<info::platform, Param>::type
  get_info() const {
    detail::unimplemented();
    return {};
  }


  /** Test if an extension is available on the platform

      \todo Should it be a param type instead of a STRING?

      \todo extend to any type of C++-string like object
  */
  bool has_extension(const string_class &extension) const {
    detail::unimplemented();
    return {};
  }


  /// Test if this platform is a host platform
  bool is_host() const {
    // Right now, this is a host-only implementation :-)
    return true;
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
