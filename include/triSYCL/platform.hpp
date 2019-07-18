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

#include "triSYCL/detail/default_classes.hpp"
#include "triSYCL/detail/global_config.hpp"

#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/info/device.hpp"
#include "triSYCL/platform/detail/host_platform.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/platform/detail/opencl_platform.hpp"
#endif
#include "triSYCL/platform/detail/platform.hpp"

namespace trisycl {

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

  // Allows the comparison operation to access the implementation
  friend shared_ptr_implementation;

public:

  // Make the implementation member directly accessible in this class
  using shared_ptr_implementation::implementation;

  /** Default constructor for platform which is the host platform

      Returns errors via the SYCL exception class.
  */
  platform() :
    shared_ptr_implementation { detail::host_platform::instance() } {}


#ifdef TRISYCL_OPENCL
  /** Construct a platform class instance using cl_platform_id of the
      OpenCL device

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the OpenCL platform.
  */
  explicit platform(cl_platform_id platform_id)
    : platform { boost::compute::platform { platform_id } } {}


  /** Construct a platform class instance using a boost::compute::platform

      This is a triSYCL extension for boost::compute interoperation.

      Return synchronous errors via the SYCL exception class.
  */
  platform(const boost::compute::platform &p)
    : shared_ptr_implementation { detail::opencl_platform::instance(p) } {}
#endif


  /**  Construct a platform object from the device selected by a device
       selector of the user's choice

       Returns errors via the SYCL exception class.
  */
  explicit platform(const device_selector &dev_selector);

#ifdef TRISYCL_OPENCL
  /** Returns the cl_platform_id of the underlying OpenCL platform

      If the platform is not a valid OpenCL platform, for example if it is
      the SYCL host, an exception is thrown

      \todo Define a SYCL exception for this
  */
  cl_platform_id get() const {
    return implementation->get();
  }


  /** Return the underlying Boost.Compute platform if it is an
      OpenCL platform

      This is a triSYCL extension
  */
  const boost::compute::platform get_boost_compute() const {
    return implementation->get_boost_compute();
  }
#endif


  /// Get the list of all the platforms available to the application
  static vector_class<platform> get_platforms() {
    // Start with the default platform
    vector_class<platform> platforms { {} };

#ifdef TRISYCL_OPENCL
    // Then add all the OpenCL platforms
    for (const auto &d : boost::compute::system::platforms())
      platforms.emplace_back(d);
#endif

    return platforms;
  }


  /** Get the OpenCL information about the requested parameter

      \todo Add to the specification
  */
  template <info::platform param>
  inline auto get_info() const;

  /// Test if an extension is available on the platform
  bool has_extension(const string_class &extension) const {
    return implementation->has_extension(extension);
  }


  /// Test if this platform is a host platform
  bool is_host() const {
    return implementation->is_host();
  }


  /** Get all the available devices for this platform

      \param[in] device_type is the device type to filter the selection
      or \c info::device_type::all by default to return all the
      devices

      \return the device list
  */
  vector_class<device>
  get_devices(info::device_type device_type = info::device_type::all) const;
};

template<>
inline auto platform::get_info<info::platform::extensions>() const {
  return implementation->get_extension_strings();
}

#define PLATFORM_GET_INFO_STRING(X)				\
template<>							\
inline auto platform::get_info<info::platform::X>() const {	\
  return implementation->get_info_string(info::platform::X);	\
}

PLATFORM_GET_INFO_STRING(profile)
PLATFORM_GET_INFO_STRING(version)
PLATFORM_GET_INFO_STRING(name)
PLATFORM_GET_INFO_STRING(vendor)

/// @} to end the execution Doxygen group

}


/* Inject a custom specialization of std::hash to have the buffer
   usable into an unordered associative container

   \todo Add this to the spec
*/
namespace std {

template <> struct hash<trisycl::platform> {

  auto operator()(const trisycl::platform &p) const {
    // Forward the hashing to the implementation
    return p.hash();
  }

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PLATFORM_HPP
