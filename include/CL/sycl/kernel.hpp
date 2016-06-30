#ifndef TRISYCL_SYCL_KERNEL_HPP
#define TRISYCL_SYCL_KERNEL_HPP

/** \file The OpenCL SYCL kernel

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
//#include "CL/sycl/info/kernel.hpp"
#include "CL/sycl/kernel/detail/kernel.hpp"
#ifdef TRISYCL_OPENCL
#include "CL/sycl/kernel/detail/opencl_kernel.hpp"
#endif

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL kernel

    \todo To be implemented

    \todo Check specification
*/
class kernel
  /* Use the underlying kernel implementation that can be shared in
     the SYCL model */
  : public detail::shared_ptr_implementation<kernel, detail::kernel> {

  // The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<kernel, detail::kernel>;

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

 public:

  /** The default object is not valid because there is no program or
      \code cl_kernel \endcode associated with it */
  kernel() = delete;

#ifdef TRISYCL_OPENCL
  /** Constructor for SYCL kernel class given an OpenCL kernel object
      with set arguments, valid for enqueuing

      Retains a reference to the \p cl_kernel object. The Caller
      should release the passed cl_kernel object when it is no longer
      needed.
  */
  kernel(cl_kernel k) : kernel { boost::compute::kernel { k } } {}


  /** Construct a kernel class instance using a boost::compute::kernel

      This is a triSYCL extension for boost::compute interoperation.

      Return synchronous errors via the SYCL exception class.
  */
  kernel(const boost::compute::kernel &k)
    : implementation_t { detail::opencl_kernel::instance(k) } {}


  /** Return the OpenCL kernel object for this kernel

      Retains a reference to the returned cl_kernel object. Caller
      should release it when finished.
  */
  cl_kernel get() const {
    return implementation->get();
  }
#endif


#if 0
  /// Return the context that this kernel is defined for
  //context get_context() const;

  /// Return the program that this kernel is part of
  //program get_program() const;

  /** Query information from the kernel object using the
      info::kernel_info descriptor.
  */
  template <info::kernel param>
  typename info::param_traits<info::kernel, param>::type
    get_info() const {
    detail::unimplemented();
  }
#endif

};

/// @} End the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_KERNEL_HPP
