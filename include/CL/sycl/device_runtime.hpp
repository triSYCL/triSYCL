#ifndef TRISYCL_SYCL_DEVICE_RUNTIME_HPP
#define TRISYCL_SYCL_DEVICE_RUNTIME_HPP

/** \file The OpenCL SYCL device-side runtime

    This code is expected to be called once some Clang & LLVM passes
    of the device compiler has been applied.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

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

/** Some function providing a cl_mem

    It is better to use a weak linkage than having this function with
    extern linkage:

    - if the device compiler is used the function is kept by the
      compiler because full link and LTO is not applied before the
      device compiler passes are applied. So there is no partial
      evaluation in the compiler according to what is returned by this
      function;

    - if the device compiler is not used, the code still links and can
      be compiled.
*/
TRISYCL_WEAK_ATTRIB_PREFIX TRISYCL_GLOBAL_AS void *TRISYCL_WEAK_ATTRIB_SUFFIX
get_some_cl_mem_hidden_in_a_buffer(const void *accessor) {
  return nullptr;
}


/// SYCL accessor seen from a device perspective
template <typename Accessor>
class accessor {

  /** The pointer to the data

      For now none of the following works yet for various reasons:
      \code
      cl::sycl::global_ptr<typename Accessor::value_type> buffer;
      cl::sycl::global<typename Accessor::value_type *> buffer;
     __global typename Accessor::pointer buffer;
     \endcode

     So use a more trivial approach
  */
  TRISYCL_GLOBAL_AS typename Accessor::value_type *buffer;

  /** The size of the accessor

      The compiler should optimize it out if it is unused */
  std::size_t size;

public:

  /// Capture the minimal accessor features
  accessor(const Accessor &a) :
    /* Initialize the buffer with actually a \c cl_mem since it will be
       overridden with the kernel argument by the serializer at
       run-time, but at least from the kernel point of view it has to
       look like an \c Accessor::value_type * for the kernel
       outlining */
    buffer {
      (TRISYCL_GLOBAL_AS typename Accessor::value_type *)
        get_some_cl_mem_hidden_in_a_buffer((const void *)&a)
    },
    size { a.get_count() }
  {}


  /// use the accessor with integers à la []
  auto &operator[](std::size_t index) const {
    return buffer[index];
  }

};


/** Set an argument value of the kernel

    \param[in] task is the implementation detail of a triSYCL kernel

    \param[in] index is the order of the argument to set (0 is the first one)

    \param[in] arg point to the argument value

    \param[in] arg_size is the size of the argument
*/
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
serialize_arg(detail::task &task,
              std::size_t index,
              void *arg,
              std::size_t arg_size) {
  std::cerr << "serialize_arg" << std::endl;
  task.set_arg(index, arg_size, arg);
}


/** The code of the kernels arranged by name

  \todo use std::byte when moving to C++17
*/
TRISYCL_WEAK_ATTRIB_PREFIX std::map<std::string, std::vector<unsigned char>>
TRISYCL_WEAK_ATTRIB_SUFFIX kernel_IR;


/** Set the kernel for a later invocation

    \param[in] task is the implementation detail of a triSYCL kernel

    \param[in] kernel_name is set by the device compiler to identify
    the kernel to call
*/
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
set_kernel(detail::task &task,
           const char *kernel_name,
           const char *kernel_short_name) {
  std::cerr << "Launch " << kernel_name << std::endl;
  std::cerr << "aka " << kernel_short_name << std::endl;
  // \todo Add kernel caching per device
  auto binary = kernel_IR[kernel_name];
  auto context = task.get_queue()->get_boost_compute().get_context();
  // Construct an OpenCL program from the precompiled kernel file
  auto program = boost::compute::program::create_with_binary(binary, context);
  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  kernel k { boost::compute::kernel { program, kernel_name } };

  task.set_kernel(k.implementation );
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
