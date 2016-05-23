#ifndef TRISYCL_SYCL_DEVICE_DETAIL_HOST_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_HOST_DEVICE_HPP

/** \file The OpenCL SYCL host device implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#include <boost/core/null_deleter.hpp>

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device/detail/device.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/platform.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** SYCL host device

    \todo The implementation is quite minimal for now. :-)
*/
class host_device : public detail::device {

public:

#ifdef TRISYCL_OPENCL
  /** Return the cl_device_id of the underlying OpenCL platform

      This throws an error since there is no OpenCL device associated
      to the host device.
  */
  cl_device_id get() const override {
    throw non_cl_error("The host device has no OpenCL device");
  }
#endif


  /// Return true since the device is a SYCL host device
  bool is_host() const override {
    return true;
  }


  /// Return false since the host device is not an OpenCL CPU device
  bool is_cpu() const override {
    return false;
  }


  /// Return false since the host device is not an OpenCL GPU device
  bool is_gpu() const override {
    return false;
  }


  /// Return false since the host device is not an OpenCL accelerator device
  bool is_accelerator() const override {
    return false;
  }


  /** Return the platform of device

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  cl::sycl::platform get_platform() const override {
    detail::unimplemented();
    return {};
  }

#if 0
  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  template <info::device Param>
  typename info::param_traits<info::device, Param>::type
  get_info() const override {
    detail::unimplemented();
    return {};
  }
#endif

  /** Specify whether a specific extension is supported on the device

      \todo To be implemented
  */
  bool has_extension(const string_class &extension) const override {
    detail::unimplemented();
    return {};
  }


  /// Get a singleton instance of the host_device
  static std::shared_ptr<host_device> instance() {
    // C++11 guaranties the static construction is thread-safe
    static host_device singleton;
    /** Use a null_deleter since the singleton should not be deleted,
        as allocated in the static area */
    static std::shared_ptr<host_device> sps { &singleton,
                                              boost::null_deleter {} };

    return sps;
  }

private:

  /// The private constructors make sure it is only constructed here
  host_device() = default;


  /// Disallow someone else to destroy the instance
  ~host_device() {}

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

#endif // TRISYCL_SYCL_DEVICE_DETAIL_HOST_DEVICE_HPP
