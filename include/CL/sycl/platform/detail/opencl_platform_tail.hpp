#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_OPENCL_PLATFORM_TAIL_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_OPENCL_PLATFORM_TAIL_HPP

/** \file The ending part of the SYCL host platform

    This is here to break a dependency between platform and device

    a-doumoulakis at gmail dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** Returns a vector class containing all SYCL devices associated with
    this OpenCL platform

    \param[in] device_type is the device type to filter the selection
    or \c info::device_type::all by default to return all the
    devices

    \return the device list
*/
vector_class<::trisycl::device> inline
opencl_platform::get_devices(const device_selector &device_selector) const {
  vector_class<::trisycl::device> devices;
  // Add the desired OpenCL devices
  for (const auto &d : get_boost_compute().devices()) {
    // Get the SYCL device from the Boost Compute device
    ::trisycl::device sycl_dev { d };
    /* Return the devices with the good criterion according to the selector.
       By calling devices on the \c boost::compute::platform we know that
       we iterate only over the device belonging to the current platform,
    */
    if (device_selector(sycl_dev) > 0)
      devices.push_back(sycl_dev);
  }

  return devices;
}

/// @} to end the Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PLATFORM_DETAIL_OPENCL_PLATFORM_TAIL_HPP
