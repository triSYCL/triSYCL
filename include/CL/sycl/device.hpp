#ifndef TRISYCL_SYCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_HPP

/** \file The OpenCL SYCL device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/device/detail/host_device.hpp"
#ifdef TRISYCL_OPENCL
#include "CL/sycl/device/detail/opencl_device.hpp"
#endif
#include "CL/sycl/info/device.hpp"
#include "CL/sycl/device_selector.hpp"
#include "CL/sycl/platform.hpp"

namespace cl {
namespace sycl {

class device_selector;
class platform;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// SYCL device
class device
  /* Use the underlying device implementation that can be shared in the
     SYCL model */
  : public detail::shared_ptr_implementation<device, detail::device> {

  // The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<device, detail::device>;

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

public:

  /// The default constructor uses the SYCL host device
  device() : implementation_t { detail::host_device::instance() } {}


#ifdef TRISYCL_OPENCL
  /** Construct a device class instance using cl_device_id of the
      OpenCL device

      Return synchronous errors via the SYCL exception class.

      Retain a reference to the OpenCL device and if this device was
      an OpenCL subdevice the device should be released by the caller
      when it is no longer needed.
  */
  device(cl_device_id device_id)
    : device { boost::compute::device { device_id } } {}


  /** Construct a device class instance using a boost::compute::device

      This is a triSYCL extension for boost::compute interoperation.

      Return synchronous errors via the SYCL exception class.
  */
  device(const boost::compute::device &d)
    : implementation_t { detail::opencl_device::instance(d) } {}
#endif


  /** Construct a device class instance using the device selector
      provided

      Return errors via C++ exception class.

      \todo
  */
  //explicit device(const device_selector &ds) : implementation { ds } {}


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



  /** Return the device_type of a device

      \todo Present in Boost.Compute, to be added to the specification
  */
  info::device_type type() const {
    if (is_host())
      return info::device_type::host;
    else if (is_cpu())
      return info::device_type::cpu;
    else if (is_gpu())
      return info::device_type::gpu;
    else if (is_accelerator())
      return info::device_type::accelerator;
    else
      // \todo Put a SYCL exception
      throw std::domain_error("Unknown cl::sycl::info::device_type");
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
    // Start with the default device
    vector_class<device> devices = { {} };

#ifdef TRISYCL_OPENCL
    // Then add all the OpenCL devices
    for (const auto &d : boost::compute::system::devices())
      devices.emplace_back(d);
#endif

    if (device_type == info::device_type::all)
      return devices;

    vector_class<device> d;
    device_type_selector s { device_type };

    // Return the devices with the good criterion according to the selector
    std::copy_if(devices.begin(), devices.end(), std::back_inserter(d),
                 [&](const device &e ) { return s(e) >= 0; });
    return d;
  }



  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo
  */
  template <typename T>
  T get_info(info::device param) const {
    //return implementation->get_info<Param>(param);
  }


  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo
  */
  template <info::device Param>
  auto get_info() const {
    // Forward to the version where the info parameter is not a template
    //return get_info<typename info::param_traits_t<info::device, Param>>(Param);
  }


  /// Test if a specific extension is supported on the device
  bool has_extension(const string_class &extension) const {
    return implementation->has_extension(extension);
  }


#ifdef XYZTRISYCL_OPENCL
  /** Partition the device into sub devices based upon the properties
      provided

      Return synchronous errors via SYCL exception classes.

      \todo
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

};

/// @} to end the Doxygen group

}
}


/* Inject a custom specialization of std::hash to have the buffer
   usable into an unordered associative container

   \todo Add this to the spec
*/
namespace std {

template <> struct hash<cl::sycl::device> {

  auto operator()(const cl::sycl::device &d) const {
    // Forward the hashing to the implementation
    return d.hash();
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

#endif // TRISYCL_SYCL_DEVICE_HPP
