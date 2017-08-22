#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_PLATFORM_TAIL_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_PLATFORM_TAIL_HPP

/** \file The ending part of of OpenCL SYCL platform

    This is here to break a dependency between platform and device

    a-doumoulakis at gmail dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** Returns a vector class containing all SYCL devices associated with
    this SYCL platform and of the \c info::device_type given.

    Return synchronous errors via SYCL exception classes.
*/
vector_class<device>
platform::get_devices(info::device_type device_type) const {

  device_type_selector s { device_type };

  /** If \c get_devices is called with the host platform
      and the right device type, returns the host_device.
  */
  if (is_host()) {
    cl::sycl::device host_dev;
    if (s(host_dev) > 0)
      return { host_dev };
    else
      return {};
  }

  vector_class<device> devices;

#ifdef TRISYCL_OPENCL
  // Add the desired OpenCL devices
  for (const auto &d : get_boost_compute().devices()) {
    // Get the SYCL device from the Boost Compute device
    cl::sycl::device sycl_dev { d };
    /* Return the devices with the good criterion according to the selector.
       By calling devices on the \c boost::compute::platform we know that
       we iterate only over the device belonging to the current platform,
    */
    if (s(sycl_dev) > 0)
      devices.push_back(sycl_dev);
  }
#endif

  return devices;
}

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

#endif // TRISYCL_SYCL_PLATFORM_DETAIL_PLATFORM_TAIL_HPP
