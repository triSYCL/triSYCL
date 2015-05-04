#ifndef TRISYCL_SYCL_DEVICE_SELECTOR_HPP
#define TRISYCL_SYCL_DEVICE_SELECTOR_HPP

/** \file The OpenCL SYCL device_selector

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** The SYCL heuristics to select a device

    The device with the highest score is selected
*/
class device_selector {

public:

  /** Returns a selected device using the functor operator defined in
      sub-classes operator()(const device &dev)

      \todo To be implemented
  */
  device select_device() const {
    detail::unimplemented();
    return {};
  }


  /**  This pure virtual operator allows the customization of device
       selection.

       It defines the behavior of the device_selector functor called by
       the SYCL runtime on device selection. It returns a "score" for each
       device in the system and the highest rated device will be used
       by the SYCL runtime.
  */
  virtual int operator() (const device &dev) const = 0;
};


/** Devices selected by heuristics of the system

    If no OpenCL device is found then it defaults to the SYCL host device.

    \todo to be implemented

    \todo to be named device_selector::default instead in the specification?
*/
class default_selector : public device_selector {

public:

  // The user-provided operator computing the score
  int operator() (const device &dev) const override {
    detail::unimplemented();
    return 1;
  }

};


/** Select devices according to device type info::device::device_type::gpu
    from all the available OpenCL devices.

    If no OpenCL GPU device is found the selector fails.

    Select the best GPU, if any.

    \todo to be implemented

    \todo to be named device_selector::gpu instead in the specification?
*/
class gpu_selector : public device_selector {

public:

  // The user-provided operator computing the score
  int operator() (const device &dev) const override {
    detail::unimplemented();
    return 1;
  }

};


/** Select devices according to device type info::device::device_type::cpu
    from all the available devices and heuristics

    If no OpenCL CPU device is found the selector fails.

    \todo to be implemented

    \todo to be named device_selector::cpu instead in the specification?
*/
class cpu_selector : public device_selector {

public:

  // The user-provided operator computing the score
  int operator() (const device &dev) const override {
    detail::unimplemented();
    return 1;
  }

};


/** Selects the SYCL host CPU device that does not require an OpenCL
    runtime

    \todo to be implemented

    \todo to be named device_selector::host instead in the specification?
*/
class host_selector : public device_selector {

public:

  // The user-provided operator computing the score
  int operator() (const device &dev) const override {
    detail::unimplemented();
    return 1;
  }

};

/// @} to end the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_SELECTOR_HPP
