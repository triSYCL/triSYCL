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
    this SYCL platform.

    Return synchronous errors via SYCL exception classes.
*/
vector_class<device>
platform::get_devices(info::device_type device_type) const {
  /** If \c get_devices is called with the host platform,
      returns the host_device.
  */
  if(is_host()) return { {} };

  vector_class<device> devices;
  device_type_selector s { device_type };

#ifdef TRISYCL_OPENCL
  // Add the desired OpenCL devices
  for (const auto &d : get_boost_compute().devices()) {
    // Get the SYCL device from the Boost Compute device
    cl::sycl::device sycl_dev { d };
    /* Return the devices with the good criterion according to the selector
       and the \c cl_platform_id of the device platform and the instance
    */
    if(s(sycl_dev) >= 0 && sycl_dev.get_platform().get() == get())
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
