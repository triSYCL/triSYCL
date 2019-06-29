#ifndef TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_TAIL_HPP
#define TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_TAIL_HPP

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

/** Get all the available devices for this platform

    \param[in] device_type is the device type to filter the selection
    or \c info::device_type::all by default to return all the
    devices

    \return the device list
*/
vector_class<::trisycl::device>
inline host_platform::get_devices(const device_selector &device_selector) const {
  /** If \c get_devices is called with the host platform
      and the right device type, returns the host_device.
  */
  if (device_selector(::trisycl::device {}) > 0)
    // Return 1 default device, i.e. the host device
    return { {} };
  else
    // No matching device
    return {};
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

#endif // TRISYCL_SYCL_PLATFORM_DETAIL_HOST_PLATFORM_TAIL_HPP
