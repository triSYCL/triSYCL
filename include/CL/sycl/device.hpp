#ifndef TRISYCL_SYCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_HPP

/** \file The OpenCL SYCL device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#include <boost/operators.hpp>

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/device/detail/host_device.hpp"
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

    Implement comparable concepts to be put into containers for
    example
 */
class device : public boost::totally_ordered<device> {

  /// The implementation forward everything to this... implementation
  std::shared_ptr<detail::device> implementation;

public:

  /** The default constructor will create an instance of the SYCL host
      device
  */
  device() : implementation { new detail::host_device } {}


#ifdef TRISYCL_OPENCL
  /** Construct a device class instance using cl_device_id of the
      OpenCL device

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the OpenCL device and if this device was
      an OpenCL subdevice the device should be released by the caller
      when it is no longer needed.
  */
  device(cl_device_id device_id) : implementation { device_id } {}
#endif


  /** Construct a device class instance using the device selector
      provided

      Return errors via C++ exception class.

      \todo
  */
  //explicit device(const device_selector &ds) : implementation { ds } {}


  /// Get the default constructors back.
  //device() = default;


#ifdef TRISYCL_OPENCL
  /** Return the cl_device_id of the underlying OpenCL platform

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the returned cl_device_id object. Caller
      should release it when finished.

      In the case where this is the SYCL host device it will throw an
      exception.
  */
  cl_device_id get() const {
    return implementation->get();
  }
#endif

  /// Return true if the device is the SYCL host device
  bool is_host() const {
    return implementation->is_host();
  }


  /// Return true if the device is an OpenCL CPU device
  bool is_cpu() const {
    return implementation->is_cpu();
  }


  /// Return true if the device is an OpenCL GPU device
  bool is_gpu() const {
    return implementation->is_gpu();
  }


  /// Return true if the device is an OpenCL accelerator device
  bool is_accelerator() const {
    return implementation->is_accelerator();
  }


  /** Return the platform of device

      Return synchronous errors via the SYCL exception class.
  */
  platform get_platform() const {
    return implementation->get_platform();
  }


  /** Return a list of all available devices

      Return synchronous errors via SYCL exception classes.
  */
  static vector_class<device>
  get_devices(info::device_type device_type = info::device_type::all) {
    return {};
  }


  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.
  */
  template <typename T>
  T get_info(info::device param) const {
    //return implementation->get_info<Param>(param);
  }


  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.
  */
  template <info::device Param>
  auto get_info() const {
    // Forward to the version where the info parameter is not a template
    //return get_info<typename info::param_traits_t<info::device, Param>>(Param);
  }


  /// Specify whether a specific extension is supported on the device
  bool has_extension(const string_class &extension) const {
    return implementation->has_extension(extension);
  }


#ifdef TRISYCL_OPENCL
  /** Partition the device into sub devices based upon the properties
      provided

      Return synchronous errors via SYCL exception classes.
  */
  vector_class<device>
  create_sub_devices(info::device_partition_type partition_type,
                     info::device_partition_property partition_property,
                     info::device_affinity_domain affinity_domain) const {
    return implementation->create_sub_devices(partition_type,
                                              partition_property,
                                              affinity_domain);
  }
#endif

  /** Equality operator

      This is generalized by boost::equality_comparable from
      boost::totally_ordered to implement the equality comparable
      concept
  */
  bool operator ==(const device &other) const {
    return implementation == other.implementation;
  }


  /** Inferior operator

      This is generalized by boost::less_than_comparable from
      boost::totally_ordered to implement the equality comparable
      concept
  */
  bool operator <(const device &other) const {
    return implementation < other.implementation;
  }

};

/// @} to end the Doxygen group

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
