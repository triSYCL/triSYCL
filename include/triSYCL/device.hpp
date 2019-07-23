#ifndef TRISYCL_SYCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_HPP

/** \file The OpenCL SYCL device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <memory>
#include <any>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/device/detail/host_device.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/device/detail/opencl_device.hpp"
#endif
#include "triSYCL/id.hpp"
#include "triSYCL/info/device.hpp"
#include "triSYCL/device_selector.hpp"
#include "triSYCL/opencl_types.hpp"
#include "triSYCL/platform.hpp"

namespace trisycl {

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

public:

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

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

      \todo Make it non-explicit in the specification?
  */
  explicit device(const device_selector &ds) {
    auto devices = device::get_devices();
    if (devices.empty())
      // \todo Put a SYCL exception
      throw std::domain_error("No device at all! Internal error...");

    /* Find the device with the best score according to the given
       device_selector */
    auto max = std::max_element(devices.cbegin(), devices.cend(),
                                [&] (const device &d1, const device &d2) {
                                  return ds(d1) < ds(d2);
                                });
    if (ds(*max) < 0)
      // \todo Put a SYCL exception
      throw std::domain_error("No device selected because no positive "
                              "device_selector score found");

    // Create the current device as a shared copy of the selected one
    implementation = max->implementation;
  }


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


  /** Return the underlying Boost.Compute device if it is an
      OpenCL device

      This is a triSYCL extension
  */
  boost::compute::device get_boost_compute() const {
    return implementation->get_boost_compute();
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
      throw std::domain_error("Unknown trisycl::info::device_type");
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
#ifdef _MSC_VER
  inline
#endif
  static vector_class<device>
    get_devices(info::device_type device_type = info::device_type::all)
    TRISYCL_WEAK_ATTRIB_SUFFIX;

  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo
  */
  /** Query the device for OpenCL info::device info

      Return synchronous errors via the SYCL exception class.

      \todo
  */
  template <info::device param>
  typename info::param_traits<info::device, param>::return_type
  get_info() const {
    return std::any_cast<
      typename info::param_traits<info::device, param>::return_type
      >(implementation->get_info(param));
  }

  /// Test if a specific extension is supported on the device
  bool has_extension(const string_class &extension) const {
    return implementation->has_extension(extension);
  }

  // Available only when prop == info::partition_property::partition_equally
  template <info::partition_property prop>
  vector_class<device> create_sub_devices(size_t nbSubDev) const
  {
    throw trisycl::feature_not_supported("unsupported feature\n");
  }

  // Available only when prop == info::partition_property::partition_by_counts
  template <info::partition_property prop>
  vector_class<device> create_sub_devices(const vector_class<size_t> &counts) const
  {
    throw trisycl::feature_not_supported("unsupported feature\n");
  }

  // Available only when prop == info::partition_property::partition_by_affinity_domain
  template <info::partition_property prop>
  vector_class<device> create_sub_devices(info::partition_affinity_domain affinityDomain) const
  {
    throw trisycl::feature_not_supported("unsupported feature\n");
  }

};

/// @} to end the Doxygen group

}


/* Inject a custom specialization of std::hash to have the buffer
   usable into an unordered associative container

   \todo Add this to the spec
*/
namespace std {

template <> struct hash<trisycl::device> {

  auto operator()(const trisycl::device &d) const {
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
