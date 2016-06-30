#ifndef TRISYCL_SYCL_KERNEL_DETAIL_KERNEL_HPP
#define TRISYCL_SYCL_KERNEL_DETAIL_KERNEL_HPP

/** \file The OpenCL SYCL kernel

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/detail/unimplemented.hpp"
//#include "CL/sycl/info/kernel.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// Abstract SYCL kernel
class kernel {

 public:

#ifdef TRISYCL_OPENCL
  /** Return the OpenCL kernel object for this kernel

      Retains a reference to the returned cl_kernel object. Caller
      should release it when finished.
  */
  virtual cl_kernel get() const = 0;
#endif


  /// Return the context that this kernel is defined for
  //virtual context get_context() const;

  /// Return the program that this kernel is part of
  //virtual program get_program() const;

  // Virtual to call the real destructor
  virtual ~kernel() {}

};

/// @} End the execution Doxygen group

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

#endif // TRISYCL_SYCL_DETAIL_KERNEL_KERNEL_HPP
