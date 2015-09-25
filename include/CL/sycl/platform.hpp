#ifndef TRISYCL_SYCL_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_HPP

/** \file The OpenCL SYCL platform

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/context.hpp"
#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/global_config.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device.hpp"
#include "CL/sycl/platform/detail/host_information.hpp"
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
class platform {

#ifdef TRISYCL_OPENCL
  /// Refer to the related OpenCL platform of any
  std::shared_ptr<boost::compute::platform> m_platform;
#endif

public:

#ifdef TRISYCL_OPENCL
  /** Construct a default platform and provide an optional error_handler
      to deals with errors

      \todo Add copy/move constructor to the implementation

      \todo Add const to the specification
  */
  explicit platform(cl_platform_id platformID) :
    m_platform { new boost::compute::platform { platformID} }
  {}
#endif

  /** Default constructor for platform

      It constructs a platform object to encapsulate the device returned
      by the default device selector.

      Returns errors via the SYCL exception class.

      Get back the default constructors, for this implementation.

      In this implementation the default platform is the host platform.
  */
  platform() = default;


  /**  Construct a platform object from the device returned by a device
       selector of the user’s choice

       Returns errors via the SYCL exception class.
  */
  explicit platform(const device_selector &devSelector) {
    detail::unimplemented();
  }


#ifdef TRISYCL_OPENCL
  /** Returns the cl_platform_id of the underlying OpenCL platform

      If the platform is not a valid OpenCL platform, for example if it is
      the SYCL host, an exception is thrown

      \todo Modify the specification to throw since returning nullptr does
      mean anything

      \todo Define a SYCL exception for this
  */
  cl_platform_id get() const {
    if (m_platform)
      return m_platform->id();

    throw std::domain_error { "The host platform is not an OpenCL platform" };
  }
#endif


  /** Get the list of all the platforms available to the application
   */
  static vector_class<platform> get_platforms() {
    // There is always the host platform at least
    vector_class<platform> platforms { platform { } };

#ifdef TRISYCL_OPENCL
    for (auto const &p : boost::compute::system::platforms())
      platforms.emplace_back(p.id());
#endif

    return platforms;
  }


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


  /** Test if an extension is available on the platform

      \todo Should it be a param type instead of a STRING?

      \todo extend to any type of C++-string like object
  */
  bool has_extension(const string_class &extension) const {
#ifdef TRISYCL_OPENCL
    if (m_platform)
      return m_platform->supports_extension(extension);
#endif
    // For now there is no supported extension on the host platform
    return false;
  }


  /// Test if this platform is a host platform
  bool is_host() const {
#ifdef TRISYCL_OPENCL
    // This is the host platform if there is no OpenCL associated
    return !m_platform;
#else
    // When there is no OpenCL, there is only a host platform
    return true;
#endif
  }


/// \todo Add to the specification and implement == and !=

/// \todo Add also < and hash for associative containers

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
