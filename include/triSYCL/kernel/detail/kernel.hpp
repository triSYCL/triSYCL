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

#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/unimplemented.hpp"
//#include "triSYCL/info/kernel.hpp"
#include "triSYCL/queue/detail/queue.hpp"
#include "triSYCL/range.hpp"

namespace trisycl::detail {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/// Abstract SYCL kernel
class kernel : detail::debug<detail::kernel> {

 public:

#ifdef TRISYCL_OPENCL
  /** Return the OpenCL kernel object for this kernel

      Retains a reference to the returned cl_kernel object. Caller
      should release it when finished.
  */
  virtual cl_kernel get() const = 0;


  /** Return the Boost.Compute OpenCL kernel object for this kernel

      This is an extension.
  */
  virtual boost::compute::kernel &get_boost_compute() = 0;
#endif


  /// Launch a single task of the kernel
  virtual void single_task(std::shared_ptr<detail::task> task,
                           std::shared_ptr<detail::queue> q) = 0;


  /** Launch a kernel with a range<>

      Do not use a template since it does not work with virtual functions

      \todo Think to a cleaner solution
  */
#define TRISYCL_ParallelForKernel_RANGE(N)                        \
  virtual void parallel_for(std::shared_ptr<detail::task> task, std::shared_ptr<detail::queue> q,     \
                            const range<N> &num_work_items) = 0;

  TRISYCL_ParallelForKernel_RANGE(1)
  TRISYCL_ParallelForKernel_RANGE(2)
  TRISYCL_ParallelForKernel_RANGE(3)
#undef TRISYCL_ParallelForKernel_RANGE


  /// Return the context that this kernel is defined for
  //virtual context get_context() const;

  /// Return the program that this kernel is part of
  //virtual program get_program() const;

  // Virtual to call the real destructor
  virtual ~kernel() {}

};

/// @} End the execution Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_KERNEL_KERNEL_HPP
