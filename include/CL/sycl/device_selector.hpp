#ifndef TRISYCL_SYCL_DEVICE_SELECTOR_HPP
#define TRISYCL_SYCL_DEVICE_SELECTOR_HPP

/** \file The OpenCL SYCL device_selector

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** The SYCL heuristics to select a device

    The device with the highest score is selected
*/
struct device_selector {
  // The user-provided operator computing the score
  virtual int operator() (device dev) = 0;
};


/** Select the best GPU, if any

    \todo to be implemented

    \todo to be named device_selector::gpu instead in the specification?

    \todo it is named opencl_gpu_selector
*/
struct gpu_selector : device_selector {
  // The user-provided operator computing the score
  int operator() (device dev) override { return 1; }
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
