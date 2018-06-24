#ifndef TRISYCL_SYCL_PLATFORM_TAIL_HPP
#define TRISYCL_SYCL_PLATFORM_TAIL_HPP

/** \file The ending part of the OpenCL SYCL platform

    This is here to break a dependence between platform and device_selector.
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {

inline vector_class<device>
platform::get_devices(info::device_type device_type) const {
  return implementation->get_devices(device_type_selector { device_type });
}

}
}

#endif
