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
#include <boost/optional.hpp>

#include "CL/sycl/detail/default_classes.hpp"

#include "CL/sycl/accessor.hpp"
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
  return (void *)23 /*nullptr*/;
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
/*  union buffer {
    TRISYCL_GLOBAL_AS typename Accessor::value_type *global;
    typename Accessor::value_type *host;
  };*/
  TRISYCL_GLOBAL_AS typename Accessor::value_type *buffer;

  /** The size of the accessor

      The compiler should optimize it out if it is unused */
  std::size_t size;

public:

  /// Capture the minimal accessor features
  accessor(Accessor &a) :
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
  {
#ifndef TRISYCL_DEVICE
    /* Register the buffer address to be updated with the final
       version before the kernel set arg */
    std::cerr << "accessor(Accessor &a) : a.register_buffer_update(buffer);"
              << (void *) &a
              << std::endl;
    std::cerr << " &buffer ="
              << (void *) &buffer
              << std::endl;
    assert(sizeof(cl_mem) == sizeof(void *));
    buffer = (int *) 456;
#ifdef TRISYCL_OPENCL
    a.implementation->register_buffer_update(buffer);
#endif
#endif
  }


  /// use the accessor with integers à la []
  auto &operator[](std::size_t index) const noexcept {
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
  std::cerr << "serialize_arg index = " << index << ", size = " << arg_size
            << ", arg = " << arg << std::endl;
  task.set_arg(index, arg_size, arg);
}


/// The binary code of the kernels
namespace code {

  /// A binary program containing some kernels
  struct program {
    /// The size of a binary program
    std::size_t binary_size;
    /// The bytes of program. Use this type for \c boost::compute
    unsigned const char *binary;

    /** The current program

        \todo Deal with several programs
    */
    static boost::optional<program> p;

    program(std::size_t binary_size, const char *binary)
      : binary_size { binary_size }
      , binary { reinterpret_cast<unsigned const char *>(binary) } {
        p = *this;
        std::cerr << "binary_size = " << binary_size << std::endl;
      }
  };

  inline boost::optional<program> program::p;

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
  std::cerr << "Setting up  " << kernel_name << std::endl;
  std::cerr << " aka " << kernel_short_name << std::endl;
  // \todo Add kernel caching per device
  // auto binary = kernel_IR.find(kernel_name);
  // if (binary == kernel_IR.end()) {
#ifdef TRISYCL_OPENCL
  auto context = task.get_queue()->get_boost_compute().get_context();
  // Construct an OpenCL program from the precompiled kernel file
  auto program = boost::compute::program::create_with_binary
    (code::program::p->binary,
     code::program::p->binary_size,
     context);
  std::cerr << "Name device " << task.get_queue()->get_boost_compute().get_device().name()
            << std::endl;
  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  cl::sycl::kernel k { boost::compute::kernel { program, kernel_short_name } };

  task.set_kernel(k.implementation);
#endif
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
