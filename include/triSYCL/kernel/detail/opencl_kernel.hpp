#ifndef TRISYCL_SYCL_KERNEL_DETAIL_OPENCL_KERNEL_HPP
#define TRISYCL_SYCL_KERNEL_DETAIL_OPENCL_KERNEL_HPP

/** \file The OpenCL SYCL kernel

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/detail/cache.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/unimplemented.hpp"
//#include "triSYCL/info/kernel.hpp"
#include "triSYCL/kernel/detail/kernel.hpp"
#include "triSYCL/queue/detail/queue.hpp"


namespace trisycl::detail {

/// An abstraction of the OpenCL kernel
class opencl_kernel : public detail::kernel,
                      detail::debug<opencl_kernel> {

  /// Use the Boost Compute abstraction of the OpenCL kernel
  boost::compute::kernel k;

  /** A cache to always return the same alive kernel for a given
      OpenCL kernel

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_kernel, detail::opencl_kernel> cache;

  opencl_kernel(const boost::compute::kernel &k) : k { k } {}

 public:

  ///// Get a singleton instance of the opencl_device
  static std::shared_ptr<opencl_kernel>
  instance(const boost::compute::kernel &k) {
    return cache.get_or_register(k.get(),
                                 [&] { return new opencl_kernel { k }; });
  }

  /** Return the underlying OpenCL object

      \todo Improve the spec to deprecate C OpenCL host API and move
      to C++ instead to avoid this ugly ownership management
  */
  cl_kernel get() const override {
    /// \todo Test error and throw. Externalize this feature in Boost.Compute?
    clRetainKernel(k);
    return k.get();
  }


  /** Return the Boost.Compute OpenCL kernel object for this kernel

      This is an extension.
  */
  boost::compute::kernel &get_boost_compute() override {
    return k;
  }


  //context get_context() const override

  //program get_program() const override

#if 0
  template <info::kernel param>
  typename info::param_traits<info::kernel, param>::type
  get_info() const {
    TRISYCL_UNIMPL;
  }
#endif


  /** Launch a single task of the OpenCL kernel

      \todo Remove either task or q
   */
  void single_task(std::shared_ptr<detail::task> task,
                   std::shared_ptr<detail::queue> q) override {
    q->get_boost_compute().enqueue_task(k);
    /* For now use a crude synchronization mechanism to map directly a
       host task to an accelerator task */
    q->get_boost_compute().finish();
  }


  /** Launch an OpenCL kernel with a range<>

      Do not use a template since it does not work with virtual functions

      \todo Think to a cleaner solution

      \todo Remove either task or q
  */
#define TRISYCL_ParallelForKernel_RANGE(N)                              \
  void parallel_for(std::shared_ptr<detail::task> task,                 \
                    std::shared_ptr<detail::queue> q,                   \
                    const range<N> &num_work_items) override {          \
    static_assert(sizeof(range<N>::value_type) == sizeof(size_t),       \
                  "num_work_items::value_type compatible with "         \
                  "Boost.Compute");                                     \
    q->get_boost_compute().enqueue_nd_range_kernel                      \
      (k,                                                               \
       static_cast<size_t>(N),                                          \
       NULL,                                                            \
       static_cast<const size_t *>(num_work_items.data()),              \
       NULL);                                                           \
    /* For now use a crude synchronization mechanism to map directly a  \
       host task to an accelerator task */                              \
    q->get_boost_compute().finish();                                    \
  };

  TRISYCL_ParallelForKernel_RANGE(1)
  TRISYCL_ParallelForKernel_RANGE(2)
  TRISYCL_ParallelForKernel_RANGE(3)
#undef TRISYCL_ParallelForKernel_RANGE


  /// Unregister from the cache on destruction
  ~opencl_kernel() override {
    cache.remove(k.get());
  }

};

/* Allocate the cache here but since this is a pure-header library,
   use a weak symbol so that only one remains when SYCL headers are
   used in different compilation units of a program
*/
TRISYCL_WEAK_ATTRIB_PREFIX
detail::cache<cl_kernel, detail::opencl_kernel> opencl_kernel::cache
TRISYCL_WEAK_ATTRIB_SUFFIX;

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_KERNEL_DETAIL_OPENCL_KERNEL_HPP
