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

/** Return a list of all available devices

    Return synchronous errors via SYCL exception classes.
*/
vector_class<device>
platform::get_devices(info::device_type device_type) const {
  // Start with the default device
  vector_class<device> devices = { {} };

#ifdef TRISYCL_OPENCL
  // Then add all the OpenCL devices
  for (const auto &d : boost::compute::system::devices())
    devices.emplace_back(d);
#endif

  // The selected devices
  vector_class<device> sd;
  device_type_selector s { device_type };

  // Return the devices with the good criterion according to the selector
  std::copy_if(devices.begin(), devices.end(), std::back_inserter(sd),
               [&](const device &e) { return s(e) >= 0; });
  return sd;
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
