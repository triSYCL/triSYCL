#ifndef TRISYCL_SYCL_HANDLER_HPP
#define TRISYCL_SYCL_HANDLER_HPP

/** \file The OpenCL SYCL command group handler

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>
#include <tuple>
#include <utility>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/accessor.hpp"
#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/detail/instantiate_kernel.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/kernel.hpp"
#include "triSYCL/opencl_types.hpp"
#include "triSYCL/parallelism.hpp"
#include "triSYCL/queue/detail/queue.hpp"

namespace trisycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** Command group handler class

    A command group handler object can only be constructed by the SYCL runtime.

    All of the accessors defined in the command group scope take as a
    parameter an instance of the command group handler and all the kernel
    invocation functions are methods of this class.
*/
class handler : public detail::debug<handler> {

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
  /** Set accessor kernel arg for an OpenCL kernel which is used through the
      SYCL/OpenCL interop interface

      The index value specifies which parameter of the OpenCL kernel is
      being set and the accessor object, which OpenCL buffer or image is
      going to be given as kernel argument.

      \todo Update the specification to use a ref && to the accessor instead?

      \todo It is not that clean to have set_arg() associated to a
      command handler. Rethink the specification?

      \todo It seems more logical to have these methods on kernel instead
  */
  template <typename DataType,
            int Dimensions,
            access::mode Mode,
            access::target Target = access::target::global_buffer>
  void set_arg(int arg_index,
               accessor<DataType, Dimensions, Mode, Target> && acc_obj) {
    /* Think about setting the kernel argument before actually calling
       the kernel.

       Explicitly capture task by copy instead of having this captured
       by reference and task by reference by side effect */
    task->add_prelude([=, task = task] {
        task->set_arg(arg_index, acc_obj.implementation->get_cl_buffer());
      });
  }


  /** Set kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interoperability interface with a wrapper type
  */
  template <typename T, typename = std::enable_if_t<is_wrapper<T>::value> >
  void set_arg(int arg_index, T && scalar_value) {
    /* Explicitly capture task by copy instead of having this captured
       by reference and task by reference by side effect */
    task->add_prelude([=, task = task] {
        task->set_arg(arg_index, scalar_value.unwrap());
      });
  }


  /** Set kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interoperability interface without a wrapper type
  */
  template <typename T>
  std::enable_if_t<!is_wrapper<T>::value>
  set_arg(int arg_index, T && scalar_value) {
    /* Explicitly capture task by copy instead of having this captured
       by reference and task by reference by side effect */
    task->add_prelude([=, task = task] {
        task->set_arg(arg_index, scalar_value);
      });
  }

private:

  /// Helper to individually call set_arg() for each argument
  template <std::size_t... Is, typename... Ts>
  void dispatch_set_arg(std::index_sequence<Is...>, Ts&&... args) {
    // Use an intermediate tuple to ease individual argument access
    auto &&t = std::make_tuple(std::forward<Ts>(args)...);
    // Dispatch individual set_arg() for each argument
    auto just_to_evaluate = {
      0 /*< At least 1 element to deal with empty set_args() */,
      ( set_arg(Is, std::forward<Ts>(std::get<Is>(t))), 0)...
    };
    // Remove the warning about unused variable
    static_cast<void>(just_to_evaluate);
  }

public:

  /** Set all kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interop interface

      \todo Update the specification to add this function according to
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15978 proposal
  */
  template <typename... Ts>
  void set_args(Ts &&... args) {
    /* Construct a set of increasing argument index to be able to call
       the real set_arg */
    dispatch_set_arg(std::index_sequence_for<Ts...>{},
                     std::forward<Ts>(args)...);
  }
#endif

private:

  /** Schedule the kernel

      Add a traced version of the kernel in host mode or add the
      kernel in an instantiating function for later extraction by the
      compiler
  */
  template <typename KernelName,
            typename Kernel>
  void schedule_kernel(Kernel k) {
    /* Explicitly capture task by copy instead of having this captured
       by reference and task by reference by side effect */
    task->schedule(detail::trace_kernel<KernelName>([=, t = task] () mutable {
          if (t->owner_queue->is_host())
            k();
          else {
            TRISYCL_DUMP_T("schedule_kernel &k = " << (void *) &k);

            // Structure the kernel code so it can be outlined and called
            detail::launch_device_kernel<KernelName>(*t, k);
            // \todo for now only deal with 1 physical work-item only
            t->get_kernel().single_task(t, t->get_queue());
          }
        }));
  }

  /** Schedule a parallel for kernel

      \todo Add host fall-back execution for parallel_for_kernel

      Add a traced version of the kernel in host mode or add the
      kernel in an instantiating function for later extraction by the
      compiler and then execute it using parallel_for
  */
  template <typename KernelName,
            typename Kernel,
            int N>
  void schedule_parallel_for_kernel(Kernel k, const range<N> &num_work_items) {
    task->schedule(detail::trace_kernel<KernelName>([=, t = task] () mutable {
          // if (t->owner_queue->is_host())
             // k();
          // else {
            TRISYCL_DUMP_T("schedule_parallel_for_kernel &k = " << (void *) &k);

            // Structure the kernel code so it can be outlined and called
            detail::launch_device_kernel<KernelName>(*t, k);

            t->get_kernel().parallel_for(t, t->get_queue(), num_work_items);
      // }
    }));
  }

public:

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
  template <typename KernelName = std::nullptr_t,
            typename ParallelForFunctor>
  void single_task(ParallelForFunctor &&f) {
    TRISYCL_DUMP_T("single_task &f = " << (void *) &f);

    schedule_kernel<KernelName>(std::forward<ParallelForFunctor>(f));
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
      Dimensions
  */
#if defined(TRISYCL_USE_OPENCL_ND_RANGE)
  #define TRISYCL_parallel_for_functor_GLOBAL(N)                        \
  template <typename KernelName = std::nullptr_t,                       \
            typename ParallelForFunctor>                                \
  void parallel_for(range<N> global_size,                               \
                    ParallelForFunctor f) {                             \
    schedule_parallel_for_kernel<KernelName>([=] {                      \
        detail::parallel_for(global_size, f);                           \
     }, global_size);                                                   \
  }
#else
  #define TRISYCL_parallel_for_functor_GLOBAL(N)                        \
  template <typename KernelName = std::nullptr_t,                       \
            typename ParallelForFunctor>                                \
  void parallel_for(range<N> global_size,                               \
                    ParallelForFunctor f) {                             \
    schedule_kernel<KernelName>([=] {                                   \
        detail::parallel_for(global_size, f);                           \
     });                                                                \
  }
#endif

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
#define TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(N)               \
  template <typename KernelName = std::nullptr_t,                 \
            typename ParallelForFunctor>                          \
  void parallel_for(range<N> global_size,                         \
                    id<N> offset,                                 \
                    ParallelForFunctor f) {                       \
    schedule_kernel<KernelName>([=] {                             \
        detail::parallel_for_global_offset(global_size,           \
                                           offset,                \
                                           f);                    \
      });                                                         \
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
  template <typename KernelName = std::nullptr_t,
            int Dimensions,
            typename ParallelForFunctor>
  void parallel_for(nd_range<Dimensions> r,
                    ParallelForFunctor f) {
    schedule_kernel<KernelName>([=] { detail::parallel_for(r, f); });
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
            int Dimensions = 1,
            typename ParallelForFunctor>
  void parallel_for_work_group(nd_range<Dimensions> r,
                               ParallelForFunctor f) {
    schedule_kernel<KernelName>([=] {
        detail::parallel_for_workgroup(r, f);
      });
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
            int Dimensions = 1,
            typename ParallelForFunctor>
  void parallel_for_work_group(range<Dimensions> r1, range<Dimensions> r2,
                               ParallelForFunctor f) {
    parallel_for_work_group(nd_range<Dimensions> { r1, r2 }, f);
  }


  /** Kernel invocation method of a kernel defined as pointer to a kernel
      object, described in detail in 3.5.3

      \todo Add in the spec a version taking a kernel and a functor,
      to have host fall-back
  */
  void single_task(kernel sycl_kernel) {
    /* For now just use the usual host task system to schedule          \
       manually the OpenCL kernels instead of using OpenCL event-based  \
       scheduling                                                       \
                                                                        \
       \todo Move the tracing inside the kernel implementation          \
                                                                        \
       \todo Simplify this 2 step ugly interface                        \
    */                                                                  \
    task->set_kernel(sycl_kernel.implementation);                       \
    task->schedule(detail::trace_kernel<kernel>([=, t = task] {         \
          sycl_kernel.implementation->single_task(t, t->get_queue());   \
        }));                                                            \
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
    /* Use an intermediate variable to capture task by copy because     \
       otherwise "this" is captured by reference and havoc with task    \
       just accessing the dead "this". Nasty bug to find... */          \
    task->schedule(detail::trace_kernel<kernel>([=, t = task] {         \
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
  template <int Dimensions = 1>
  void parallel_for(nd_range<Dimensions>, kernel syclKernel) {
    TRISYCL_UNIMPL;
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

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_HANDLER_HPP
