#ifndef TRISYCL_SYCL_DEVICE_HPP
#define TRISYCL_SYCL_DEVICE_HPP

/** \file The OpenCL SYCL device

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL device

    \todo The implementation is quite minimal for now. :-)
*/
struct device {
  device() {}
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

#endif // TRISYCL_SYCL_DEVICE_HPP
