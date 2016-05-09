#ifndef TRISYCL_SYCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_HPP

/** \file The OpenCL SYCL device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/device.hpp"
#include "CL/sycl/platform.hpp"

namespace cl {
namespace sycl {

class device_selector;
class platform;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL device

    \todo The implementation is quite minimal for now. :-)
*/
class device {

public:

#ifdef TRISYCL_OPENCL
  /** Construct a device class instance using cl_device_id of the OpenCL
      device

      Return synchronous errors via the SYCL exception
      class.

      Retain a reference to the OpenCL device and if this device was an
      OpenCL subdevice the device should be released by the caller when it
      is no longer needed.

      \todo To be implemented
  */
  explicit device(cl_device_id deviceId) {
    detail::unimplemented();
  }
#endif


  /** Construct a device class instance using the device selector
      provided

      Return errors via C++ exception class.

      \todo To be implemented
  */
  explicit device(const device_selector &deviceSelector) {
    detail::unimplemented();
  }


  /** The default constructor will create an instance of the SYCL host
      device

      Get the default constructors back.
  */
  device() = default;


#ifdef TRISYCL_OPENCL
  /** Return the cl_device_id of the underlying OpenCL platform

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the returned cl_device_id object. Caller
      should release it when finished.

      In the case where this is the SYCL host device it will return a
      nullptr.

      \todo To be implemented
  */
  cl_device_id get() const {
    detail::unimplemented();
    return {};
  }
#endif

  /** Return true if the device is a SYCL host device

      \todo To be implemented
  */
  bool is_host() const {
    detail::unimplemented();
    return true;
  }


  /** Return true if the device is an OpenCL CPU device

      \todo To be implemented
  */
  bool is_cpu() const {
    detail::unimplemented();
    return {};
  }


  /** Return true if the device is an OpenCL GPU device

      \todo To be implemented
  */
  bool is_gpu() const {
    detail::unimplemented();
    return {};
  }


  /** Return true if the device is an OpenCL accelerator device

      \todo To be implemented
  */
  bool is_accelerator() const {
    detail::unimplemented();
    return {};
  }


  /** Return the platform of device

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  platform get_platform() const {
    detail::unimplemented();
    return {};
  }


  /** Return a list of all available devices

      Return synchronous errors via SYCL exception classes.

      \todo To be implemented
  */
  static vector_class<device>
  get_devices(info::device_type deviceType = info::device_type::all) {
    detail::unimplemented();
    return {};
  }


  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  template <info::device Param>
  typename info::param_traits<info::device, Param>::type
  get_info() const {
    detail::unimplemented();
    return {};
  }


  /** Specify whether a specific extension is supported on the device.

      \todo To be implemented
  */
  bool has_extension(const string_class &extension) const {
    detail::unimplemented();
    return {};
  }


  /** Partition the device into sub devices based upon the properties
      provided

      Return synchronous errors via SYCL exception classes.

      \todo To be implemented
  */
  vector_class<device>
  create_sub_devices(info::device_partition_type partitionType,
                     info::device_partition_property partitionProperty,
                     info::device_affinity_domain affinityDomain) const {
    detail::unimplemented();
    return {};
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

#endif // TRISYCL_SYCL_DEVICE_HPP
