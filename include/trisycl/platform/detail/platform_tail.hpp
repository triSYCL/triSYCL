#ifndef TRISYCL_SYCL_PLATFORM_TAIL_HPP
#define TRISYCL_SYCL_PLATFORM_TAIL_HPP

/** \file The ending part of the OpenCL SYCL platform

    This is here to break a dependence between platform and device_selector.
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl {

inline
platform::platform(const device_selector &device_selector) {
  platform host_platform = {};
#ifdef TRISYCL_OPENCL
  if (host_platform.implementation->get_devices(device_selector).empty()) {
    for (const auto &d : boost::compute::system::platforms()) {
      auto clplatform = ::trisycl::platform { d };
      auto devices = clplatform.implementation->get_devices(device_selector);
      if (!devices.empty()) {
        *this = std::move(clplatform);
        return;
      }
    }
  }
#endif
  *this = std::move(host_platform);
}

inline vector_class<device>
platform::get_devices(info::device_type device_type) const {
  return implementation->get_devices(device_type_selector { device_type });
}

}

#endif
