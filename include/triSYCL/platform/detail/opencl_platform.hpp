#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_OPENCL_PLATFORM_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_OPENCL_PLATFORM_HPP

/** \file The OpenCL triSYCL OpenCL platform implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include <memory>

#include <boost/compute.hpp>

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/detail/cache.hpp"
#include "triSYCL/device.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/platform/detail/platform.hpp"

namespace trisycl {

class device;

namespace detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// SYCL OpenCL platform
class opencl_platform : public detail::platform {

  /// Use the Boost Compute abstraction of the OpenCL platform
  boost::compute::platform p;

  /** A cache to always return the same live platform for a given OpenCL
      platform

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_platform_id, detail::opencl_platform> cache;

public:

  /// Return the cl_platform_id of the underlying OpenCL platform
  cl_platform_id get() const override {
    return p.id();
  }


  /// Return the underlying Boost.Compute platform
  const boost::compute::platform &get_boost_compute() const override {
    return p;
  }


  /// Return false since an OpenCL platform is not the SYCL host platform
  bool is_host() const override {
    return false;
  }


  /// Returning the information string parameters for the OpenCL platform
  string_class get_info_string(info::platform param) const override {
    /* Use the fact that the triSYCL info values are the same as the
       OpenCL ones used in Boost.Compute to just cast the enum class
       to the int value */
    return p.get_info<std::string>(static_cast<cl_platform_info>(param));
  }

  vector_class<string_class> get_extension_strings() const override {
    return p.extensions();
  }


  /// Specify whether a specific extension is supported on the platform
  bool has_extension(const string_class &extension) const override {
    return p.supports_extension(extension);
  }


  ///// Get a singleton instance of the opencl_platform
  static std::shared_ptr<opencl_platform>
  instance(const boost::compute::platform &p) {
    return cache.get_or_register(p.id(),
                                 [&] { return new opencl_platform { p }; });
  }


  /** Get all the available devices for this OpenCL platform

      \param[in] device_type is the device type to filter the selection
      or \c info::device_type::all by default to return all the
      devices

      \return the device list
  */
  vector_class<trisycl::device>
  get_devices(const device_selector &device_selector) const override;

private:

  /// Only the instance factory can built it
  opencl_platform(const boost::compute::platform &p) : p { p } {}

public:

  /// Unregister from the cache on destruction
  ~opencl_platform() override {
    cache.remove(p.id());
  }

};

/* Allocate the cache here but since this is a pure-header library,
   use a weak symbol so that only one remains when SYCL headers are
   used in different compilation units of a program
*/
TRISYCL_WEAK_ATTRIB_PREFIX
detail::cache<cl_platform_id, detail::opencl_platform> opencl_platform::cache
TRISYCL_WEAK_ATTRIB_SUFFIX;

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
