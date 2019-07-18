#ifndef TRISYCL_SYCL_DEVICE_RUNTIME_HPP
#define TRISYCL_SYCL_DEVICE_RUNTIME_HPP

/** \file The OpenCL SYCL device-side runtime

    This code is expected to be called once some Clang & LLVM passes
    of the device compiler has been applied.

    \todo Refactor to be more modular and OpenCL-independent so it can
    more easily be retargeted to some other runtime API.

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

#include "triSYCL/detail/default_classes.hpp"

#include "triSYCL/accessor.hpp"
#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/device/detail/host_device.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/device/detail/opencl_device.hpp"
#endif
#include "triSYCL/info/device.hpp"
#include "triSYCL/device_selector.hpp"
#include "triSYCL/platform.hpp"

namespace trisycl::drt {
/** \addtogroup device_runtime Device-side runtime implementation
    @{
*/

/// The device-side runtime
namespace drt {

/// SYCL accessor seen from a device perspective
template <typename Accessor>
class accessor :
    public detail::container_element_aspect<typename Accessor::value_type> {

  /** The pointer to the data

      For now none of the following works yet for various reasons:
      \code
      trisycl::global_ptr<typename Accessor::value_type> buffer;
      trisycl::global<typename Accessor::value_type *> buffer;
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
  accessor(Accessor &a) :
    /* Initialize the buffer with actually a \c cl_mem since it will be
       overridden with the kernel argument by the serializer at
       run-time, but at least from the kernel point of view it has to
       look like an \c Accessor::value_type * for the kernel
       outlining */
    buffer {
      reinterpret_cast<TRISYCL_GLOBAL_AS typename Accessor::value_type *>
        (a.implementation->get_order())
        },
    size { a.get_count() }
  {
#ifndef TRISYCL_DEVICE
    /* Register the buffer address to be updated with the final
       version before the kernel set arg */
    TRISYCL_DUMP_T("drt::accessor(Accessor &a) : &a = "
                   << (void *) &a << '\n'
                   << "\t&buffer = " << (void *) &buffer
                   << ", Order = " << a.implementation->get_order());
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

    \param[in] arg points to the argument value

    \param[in] arg_size is the size of the argument
*/
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
serialize_arg(detail::task &task,
              std::size_t index,
              void *arg,
              std::size_t arg_size) {
  TRISYCL_DUMP_T("serialize_arg index = " << index
                 << ", size = " << arg_size << ", arg = " << arg);
  task.set_arg(index, arg_size, arg);
}


/** Set an accessor argument of the kernel

    \param[in] task is the implementation detail of a triSYCL kernel

    \param[in] index is the order of the argument to set (0 is the first one)

    \param[in] arg points to the argument value

    \param[in] arg_size is the size of the argument
*/
TRISYCL_WEAK_ATTRIB_PREFIX void TRISYCL_WEAK_ATTRIB_SUFFIX
serialize_accessor_arg(detail::task &task,
                       std::size_t index,
                       void *arg,
                       std::size_t arg_size) {
  TRISYCL_DUMP_T("serialize_accessor_arg index = " << index
                 << ", size = " << arg_size << ", arg = " << arg);
#ifdef TRISYCL_OPENCL
  /* The address of the argument has been actually initialized by the
     accessor constructor to the accessor order to relate an argument
     to its accessor */
  task.set_arg(index,
               task.get_compute_buffer(reinterpret_cast<std::size_t>(arg)));
#endif
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
        TRISYCL_DUMP_T("Create program with binary size = 0x" << binary_size);
      }
  };

  TRISYCL_WEAK_ATTRIB_PREFIX boost::optional<program>
  TRISYCL_WEAK_ATTRIB_SUFFIX program::p;

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
  TRISYCL_DUMP_T("set_kernel setting up " << kernel_name
                 << "\n\taka " << kernel_short_name);
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
  TRISYCL_DUMP_T("...on device with name "
                 << task.get_queue()->get_boost_compute().get_device().name());
  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  trisycl::kernel k { boost::compute::kernel { program, kernel_short_name } };

  task.set_kernel(k.implementation);
#endif
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

#endif // TRISYCL_SYCL_DEVICE_RUNTIME_HPP
