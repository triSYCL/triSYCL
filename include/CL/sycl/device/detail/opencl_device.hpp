#ifndef TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_HPP

/** \file The SYCL OpenCL device implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#include <boost/compute.hpp>

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/detail/cache.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device/detail/device.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/platform.hpp"

#ifndef WEAK_ATTRIB_PREFIX
  #ifdef _MSC_VER
    #define WEAK_ATTRIB_PREFIX __declspec(selectany)
    #define WEAK_ATTRIB_SUFFIX
  #else
    #define WEAK_ATTRIB_PREFIX
    #define WEAK_ATTRIB_SUFFIX __attribute__((weak))
  #endif
#endif 

namespace cl {
namespace sycl {
namespace detail {

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


  /// Return false since an OpenCL device is not the SYCL host device
  bool is_host() const override {
    return false;
  }


  /// Test if the OpenCL is a CPU device
  bool is_cpu() const override {
    return d.type() == boost::compute::device::cpu;
  }


  /// Test if the OpenCL is a GPU device
  bool is_gpu() const override {
    return d.type() == boost::compute::device::gpu;
  }


  /// Test if the OpenCL is an accelerator device
  bool is_accelerator() const override {
    return d.type() == boost::compute::device::accelerator;
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

  /** Specify whether a specific extension is supported on the device.

      \todo To be implemented
  */
  bool has_extension(const string_class &extension) const override {
    detail::unimplemented();
    return {};
  }


  ///// Get a singleton instance of the opencl_device
  static std::shared_ptr<opencl_device>
  instance(const boost::compute::device &d) {
    return cache.get_or_register(d.id(),
                                 [&] { return new opencl_device { d }; });
  }

private:

  /// Only the instance factory can built it
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
WEAK_ATTRIB_PREFIX
detail::cache<cl_device_id, detail::opencl_device> opencl_device::cache
WEAK_ATTRIB_SUFFIX;

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

#endif // TRISYCL_SYCL_DEVICE_DETAIL_OPENCL_DEVICE_HPP
