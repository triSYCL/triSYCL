#ifndef TRISYCL_SYCL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_HPP

/** \file The OpenCL SYCL context

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/device_selector.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL context

    The implementation is quite minimal for now. :-)
*/
struct context {
  context() {}

  // \todo fix this implementation
  context(const gpu_selector &s) {}

  context(const device_selector &s) {}
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

#endif // TRISYCL_SYCL_CONTEXT_HPP
