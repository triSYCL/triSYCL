#ifndef TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_HPP

/** \file The SYCL OpenCL device implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#include <boost/compute.hpp>

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/detail/cache.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/device/detail/device.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/platform.hpp"

namespace trisycl::detail {

/// SYCL OpenCL device
class opencl_device : public detail::device {

  /// Use the Boost Compute abstraction of the OpenCL device
  boost::compute::device d;

  /** A cache to always return the same alive device for a given
      OpenCL device

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_device_id, detail::opencl_device> cache;

public:

  /// Return the cl_device_id of the underlying OpenCL device
  cl_device_id get() const override {
    return d.id();
  }


  /// Return the underlying Boost.Compute device
  boost::compute::device &get_boost_compute() override {
    return d;
  }


  /// Return false since an OpenCL device is not the SYCL host device
  bool is_host() const override {
    return false;
  }


  /// Test if the OpenCL is a CPU device
  bool is_cpu() const override {
    // Even in Boost.Compute the type is a bit-field, so use & instead of ==
    return d.type() & boost::compute::device::cpu;
  }


  /// Test if the OpenCL is a GPU device
  bool is_gpu() const override {
    // Even in Boost.Compute the type is a bit-field, so use & instead of ==
    return d.type() & boost::compute::device::gpu;
  }


  /// Test if the OpenCL is an accelerator device
  bool is_accelerator() const override {
    // Even in Boost.Compute the type is a bit-field, so use & instead of ==
    return d.type() & boost::compute::device::accelerator;
  }


  /** Return the platform of device

      Return synchronous errors via the SYCL exception class.
  */
  trisycl::platform get_platform() const override {
    return d.platform();
  }

  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.
  */
  inline std::any get_info(info::device param) const override;

  /** Specify whether a specific extension is supported on the device.
  */
  bool has_extension(const string_class &extension) const override {
    return d.supports_extension(extension);
  }


  ///// Get a singleton instance of the opencl_device
  static std::shared_ptr<opencl_device>
  instance(const boost::compute::device &d) {
    return cache.get_or_register(d.id(),
                                 [&] { return new opencl_device { d }; });
  }

private:

  template <info::device param>
  inline std::any get_info() const;
  /// Only the instance factory can build it
  opencl_device(const boost::compute::device &d) : d { d } {}

public:

  /// Unregister from the cache on destruction
  ~opencl_device() override {
    cache.remove(d.id());
  }

};

/* Allocate the cache here but since this is a pure-header library,
   use a weak symbol so that only one remains when SYCL headers are
   used in different compilation units of a program
*/
TRISYCL_WEAK_ATTRIB_PREFIX
detail::cache<cl_device_id, detail::opencl_device> opencl_device::cache
TRISYCL_WEAK_ATTRIB_SUFFIX;

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_HPP
