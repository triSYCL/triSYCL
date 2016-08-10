#ifndef TRISYCL_SYCL_HANDLER_HPP
#define TRISYCL_SYCL_HANDLER_HPP

/** \file The OpenCL SYCL command group handler

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "CL/sycl/accessor.hpp"
#include "CL/sycl/command_group/detail/task.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/kernel.hpp"
#include "CL/sycl/parallelism/detail/parallelism.hpp"
#include "CL/sycl/queue/detail/queue.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** Command group handler class

    A command group handler object can only be constructed by the SYCL runtime.

    All of the accessors defined in the command group scope take as a
    parameter an instance of the command group handler and all the kernel
    invocation functions are methods of this class.
*/
class handler {

public:

  /** Attach the task and accessors to it.
   */
  std::shared_ptr<detail::task> task;


  /* Create a command group handler from the queue detail

     The queue detail is used to track kernel completion.

     Note that this is an implementation dependent constructor. Normal
     users cannot construct handler from scratch.

     \todo Make this constructor private
  */
  handler(const std::shared_ptr<detail::queue> &q) {
    // Create a new task for this command_group
    task = std::make_shared<detail::task>(q);
  }


#ifdef TRISYCL_OPENCL
  /** Set kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interop interface

      The index value specifies which parameter of the OpenCL kernel is
      being set and the accessor object, which OpenCL buffer or image is
      going to be given as kernel argument.

      \todo Update the specification to use a ref to the accessor instead?

      \todo add a variadic method that accepts accessors too

      \todo It is not that clean to have set_arg() associated to a
      command handler. Rethink the specification?

      \todo It seems more logical to have these methods on kernel instead
  */
  template <typename DataType,
            std::size_t Dimensions,
            access::mode Mode,
            access::target Target = access::target::global_buffer>
  void set_arg(int arg_index,
               accessor<DataType, Dimensions, Mode, Target> acc_obj) {
    /* Before running the kernel, make sure the cl_mem behind this
       accessor is up-to-date on the device if needed and pass it to
       the kernel.

       Explicitly capture task by copy instead of having this captured
       by reference and task by reference by side effect */
    task->add_prelude([=, task = task] {
        acc_obj.implementation->copy_in_cl_buffer();
        task->get_kernel().get_boost_compute()
          .set_arg(arg_index, acc_obj.implementation->get_cl_buffer());
      });
    /* After running the kernel, make sure the cl_mem behind this
       accessor is up-to-date on the host if needed */
    task->add_postlude([=] {
        acc_obj.implementation->copy_back_cl_buffer();
      });
  }
#endif


  /** Set kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interoperability interface

      The index value specifies which parameter of the OpenCL kernel is
      being set and the accessor object, which OpenCL buffer or image is
      going to be given as kernel argument.

      \todo It is not that clean to have set_arg() associated to a
      command handler. Rethink the specification?

      \todo To be implemented
  */
  template <typename T>
  void set_arg(int arg_index, T scalar_value) {
    detail::unimplemented();
  }


  /** Kernel invocation method of a kernel defined as a lambda or
      functor. If it is a lambda function or the functor type is globally
      visible there is no need for the developer to provide a kernel name type
      (typename KernelName) for it, as described in 3.5.3

      SYCL single_task launches a computation without parallelism at
      launch time.

      \param F specify the kernel to be launched as a single_task

      \param KernelName is a class type that defines the name to be used for
      the underlying kernel
  */
  template <typename KernelName = std::nullptr_t>
  void single_task(std::function<void(void)> F) {
    task->schedule(detail::trace_kernel<KernelName>(F));
  }


  /** SYCL parallel_for launches a data parallel computation with
      parallelism specified at launch time by a range<>

      Kernel invocation method of a kernel defined as a lambda or functor,
      for the specified range and given an id or item for indexing in the
      indexing space defined by range.

      If it is a lambda function or the if the functor type is globally
      visible there is no need for the developer to provide a kernel name
      type (typename KernelName) for it, as described in detail in 3.5.3

      \param global_size is the full size of the range<>

      \param N dimensionality of the iteration space

      \param f is the kernel functor to execute

      \param KernelName is a class type that defines the name to be used
      for the underlying kernel

      Unfortunately, to have implicit conversion to work on the range, the
      function can not be templated, so instantiate it for all the
      dimensions
  */
#define TRISYCL_parallel_for_functor_GLOBAL(N)                          \
  template <typename KernelName = std::nullptr_t,                       \
            typename ParallelForFunctor>                                \
  void parallel_for(range<N> global_size,                               \
                    ParallelForFunctor f) {                             \
    task->schedule(detail::trace_kernel<KernelName>([=] {               \
          detail::parallel_for(global_size, f);                         \
        }));                                                            \
  }

  TRISYCL_parallel_for_functor_GLOBAL(1)
  TRISYCL_parallel_for_functor_GLOBAL(2)
  TRISYCL_parallel_for_functor_GLOBAL(3)


  /** Kernel invocation method of a kernel defined as a lambda or functor,
      for the specified range and offset and given an id or item for
      indexing in the indexing space defined by range

      If it is a lambda function or the if the functor type is globally
      visible there is no need for the developer to provide a kernel name
      type (typename KernelName) for it, as described in detail in 3.5.3

      \param global_size is the global size of the range<>

      \param offset is the offset to be add to the id<> during iteration

      \param f is the kernel functor to execute

      \param ParallelForFunctor is the kernel functor type

      \param KernelName is a class type that defines the name to be used for
      the underlying kernel

      Unfortunately, to have implicit conversion to work on the range, the
      function can not be templated, so instantiate it for all the
      dimensions
  */
#define TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(N)       \
  template <typename KernelName = std::nullptr_t,         \
            typename ParallelForFunctor>                  \
  void parallel_for(range<N> global_size,                 \
                    id<N> offset,                         \
                    ParallelForFunctor f) {               \
    task->schedule(detail::trace_kernel<KernelName>([=] { \
          detail::parallel_for_global_offset(global_size, \
                                             offset,      \
                                             f);          \
        }));                                              \
  }

  TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(1)
  TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(2)
  TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(3)


  /** Kernel invocation method of a kernel defined as a lambda or functor,
      for the specified nd_range and given an nd_item for indexing in the
      indexing space defined by the nd_range

      If it is a lambda function or the if the functor type is globally
      visible there is no need for the developer to provide a kernel name
      type (typename KernelName) for it, as described in detail in 3.5.3

      \param r defines the iteration space with the work-group layout and
      offset

      \param Dimensions dimensionality of the iteration space

      \param f is the kernel functor to execute

      \param ParallelForFunctor is the kernel functor type

      \param KernelName is a class type that defines the name to be used for
      the underlying kernel
  */
  template <typename KernelName,
            std::size_t Dimensions,
            typename ParallelForFunctor>
  void parallel_for(nd_range<Dimensions> r, ParallelForFunctor f) {
    task->schedule(detail::trace_kernel<KernelName>([=] {
          detail::parallel_for(r, f);
        }));
  }


  /** Hierarchical kernel invocation method of a kernel defined as a
      lambda encoding the body of each work-group to launch

      May contain multiple kernel built-in parallel_for_work_item
      functions representing the execution on each work-item.

      Launch num_work_groups work-groups of runtime-defined
      size. Described in detail in 3.5.3.

      \param r defines the iteration space with the work-group layout and
      offset

      \param Dimensions dimensionality of the iteration space

      \param f is the kernel functor to execute

      \param ParallelForFunctor is the kernel functor type

      \param KernelName is a class type that defines the name to be used for
      the underlying kernel
  */
  template <typename KernelName = std::nullptr_t,
            std::size_t Dimensions = 1,
            typename ParallelForFunctor>
  void parallel_for_work_group(nd_range<Dimensions> r,
                               ParallelForFunctor f) {
    task->schedule(detail::trace_kernel<KernelName>([=] {
          detail::parallel_for_workgroup(r, f); }));
  }


  /** Kernel invocation method of a kernel defined as pointer to a kernel
      object, described in detail in 3.5.3

      \todo Add in the spec a version taking a kernel and a functor,
      to have host fall-back

      \todo To be implemented
  */
  void single_task(kernel syclKernel) {
    detail::unimplemented();
  }


  /** Kernel invocation method of a kernel defined as a kernel object,
      for the specified range and given an id or item for indexing in
      the indexing space defined by range, described in detail in
      5.4.

      \todo Add in the spec a version taking a kernel and a functor,
      to have host fall-back
  */
#define TRISYCL_ParallelForKernel_RANGE(N)                              \
  void parallel_for(range<N> num_work_items,                            \
                    kernel sycl_kernel) {                               \
    /* For now just use the usual host task system to schedule          \
       manually the OpenCL kernels instead of using OpenCL event-based  \
       scheduling                                                       \
                                                                        \
       \todo Move the tracing inside the kernel implementation          \
                                                                        \
       \todo Simplify this 2 step ugly interface                        \
    */                                                                  \
    task->set_kernel(sycl_kernel.implementation);                       \
    std::cerr << "handler task" << (void *) task.get()  << std::endl;\
    std::cerr << "handler parallel_for q" << (void *) task->get_queue().get() << std::endl;\
    /* Use an intermediate variable to capture task by copy because     \
       otherwise it was this which is captured by reference and havoc   \
       happens. Nasty bug to find... */                                 \
    auto &t = task;                                                     \
    task->schedule(detail::trace_kernel<kernel>([=] {                   \
              std::cerr << "capture task" << (void *) t.get()  << std::endl;\
    std::cerr << "capture q" << (void *) t->get_queue().get() << std::endl;\
          sycl_kernel.implementation->parallel_for(t, t->get_queue(),   \
                                                   num_work_items); })); \
  }

  /* Do not use a template parameter since otherwise the parallel_for
     functor is selected instead of this one

     \todo Clean this
  */
  TRISYCL_ParallelForKernel_RANGE(1)
  TRISYCL_ParallelForKernel_RANGE(2)
  TRISYCL_ParallelForKernel_RANGE(3)
#undef TRISYCL_ParallelForKernel_RANGE

  /** Kernel invocation method of a kernel defined as pointer to a kernel
      object, for the specified nd_range and given an nd_item for indexing
      in the indexing space defined by the nd_range, described in detail
      in 3.5.3

      \todo Add in the spec a version taking a kernel and a functor,
      to have host fall-back

      \todo To be implemented
  */
  template <std::size_t Dimensions = 1>
  void parallel_for(nd_range<Dimensions>, kernel syclKernel) {
    detail::unimplemented();
  }

};

namespace detail {

/** Register a buffer as used by a task

    This is a proxy function to avoid complicated type recursion.
*/
static std::shared_ptr<detail::task>
add_buffer_to_task(handler *command_group_handler,
                   std::shared_ptr<detail::buffer_base> b,
                   bool is_write_mode) {
  command_group_handler->task->add_buffer(b, is_write_mode);
  return command_group_handler->task;
}

}

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

#endif // TRISYCL_SYCL_HANDLER_HPP
