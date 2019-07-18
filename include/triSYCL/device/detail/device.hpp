#ifndef TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_HPP

/** \file The OpenCL SYCL abstract device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/platform.hpp"

namespace trisycl::detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// An abstract class representing various models of SYCL devices
class device {

public:

#ifdef TRISYCL_OPENCL
  /// Return the cl_device_id of the underlying OpenCL platform
  virtual cl_device_id get() const = 0;

  /// Return the underlying Boost.Compute device, if any
  virtual boost::compute::device &get_boost_compute() = 0;
#endif


  /// Return true if the device is a SYCL host device
  virtual bool is_host() const = 0;


  /// Return true if the device is an OpenCL CPU device
  virtual bool is_cpu() const = 0;


  /// Return true if the device is an OpenCL GPU device
  virtual bool is_gpu() const = 0;


  /// Return true if the device is an OpenCL accelerator device
  virtual bool is_accelerator() const = 0;


  /// Return the platform of device
  virtual trisycl::platform get_platform() const = 0;


  /// Query the device for OpenCL info::device info
  virtual std::any get_info(info::device param) const = 0;


  /// Specify whether a specific extension is supported on the device.
  virtual bool has_extension(const string_class &extension) const = 0;


  // Virtual to call the real destructor
  virtual ~device() {}

};

/// @} to end the execution Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_HPP
