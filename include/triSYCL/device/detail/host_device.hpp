#ifndef TRISYCL_SYCL_DEVICE_DETAIL_HOST_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_DETAIL_HOST_DEVICE_HPP

/** \file The OpenCL SYCL host device implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/detail/singleton.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/device/detail/device.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/platform.hpp"

namespace trisycl::detail {

/** SYCL host device

    \todo The implementation is quite minimal for now. :-)
*/
class host_device : public detail::device,
                    public detail::singleton<host_device> {

public:

#ifdef TRISYCL_OPENCL
  /** Return the cl_device_id of the underlying OpenCL platform

      This throws an error since there is no OpenCL device associated
      to the host device.
  */
  cl_device_id get() const override {
    throw non_cl_error("The host device has no OpenCL device");
  }


  /** Return the underlying Boost.Compute device

      This throws an error since there is no OpenCL device associated
      to the host device.
  */
  boost::compute::device &get_boost_compute() override {
    throw non_cl_error("The host device has no underlying OpenCL device");
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
  trisycl::platform get_platform() const override {
    TRISYCL_UNIMPL;
    return {};
  }

  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
#define TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(name, result) \
  case (info::device::name) : return (result);
  std::any
  get_info(info::device param) const override {
    switch (param) {
    TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(max_work_group_size, static_cast<std::size_t>(8))
    TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(max_work_item_sizes, (trisycl::id<3>{ 128, 128, 128 }))
    TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(max_compute_units, static_cast<trisycl::cl_uint>(8))
    TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(device_type, info::device_type::host)
    TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(local_mem_type, info::local_mem_type::global)
    TRISYCL_DEFINE_DEVICE_HOST_INFO_TEMPLATE(local_mem_size, static_cast<trisycl::cl_ulong>(32768))
    default:
      return 0;
    }
  }

  /** Specify whether a specific extension is supported on the device

      \todo To be implemented
  */
  bool has_extension(const string_class &extension) const override {
    TRISYCL_UNIMPL;
    return {};
  }


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

#endif // TRISYCL_SYCL_DEVICE_DETAIL_HOST_DEVICE_HPP
