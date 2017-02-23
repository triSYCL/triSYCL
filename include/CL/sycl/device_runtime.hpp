#ifndef TRISYCL_SYCL_DEVICE_RUNTIME_HPP
#define TRISYCL_SYCL_DEVICE_RUNTIME_HPP

/** \file The OpenCL SYCL device-side runtime

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/command_group/detail/task.hpp"
#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/device/detail/host_device.hpp"
#ifdef TRISYCL_OPENCL
#include "CL/sycl/device/detail/opencl_device.hpp"
#endif
#include "CL/sycl/info/device.hpp"
#include "CL/sycl/device_selector.hpp"
#include "CL/sycl/platform.hpp"

namespace cl {
namespace sycl {

/** \addtogroup device_runtime Device-side runtime implementation
    @{
*/

/// The device-side runtime
namespace drt {

/// SYCL accessor seen from a device perspective
template <typename T>
class accessor {

  /// The pointer to the data
  typename T::pointer buffer;

  /** The size of the accessor

      The compiler should optimize it out if it is unused */
  std::size_t size;

public:

  /// Capture the minimal accessor features
  accessor(const T &a) :
    buffer { const_cast<typename T::pointer>(a.get_pointer()) },
    size { a.get_count() } {}


  /// Use the accessor with integers à la []
  auto &operator[](std::size_t index) const {
    return buffer[index];
  }

};


/** Set an argument value of the kernel

    \param[in] index is the order of the argument to set (0 is the first one)

    \param[in] arg point to the argument value

    \param[in] arg_size is the size of the argument
*/
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
serialize_arg(detail::task &task,
              std::size_t index,
              void *arg,
              std::size_t arg_size) {
  task.get_kernel().get_boost_compute().set_arg(index, arg_size, arg);
}

/// @} to end the Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_RUNTIME_HPP
