#ifndef TRISYCL_SYCL_HANDLER_HPP
#define TRISYCL_SYCL_HANDLER_HPP

/** \file The OpenCL SYCL command group handler

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/accessor.hpp"
#include "CL/sycl/command_group/detail/task.hpp"
#include "CL/sycl/parallelism/detail/parallelism.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/exception.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** Kernel

    \todo To be implemented
*/
class kernel {
};


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
  std::shared_ptr<detail::task> current_task;


  handler() {
    // Create a new task for this command_group
    current_task = std::make_shared<detail::task>();
  }


  /** Set kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interop interface

      The index value specifies which parameter of the OpenCL kernel is
      being set and the accessor object, which OpenCL buffer or image is
      going to be given as kernel argument.

      \todo To be implemented
  */
  template <typename DataType,
            std::size_t Dimensions,
            access::mode Mode,
            access::target Target = access::global_buffer>
  void set_arg(int arg_index,
               accessor<DataType, Dimensions, Mode, Target> acc_obj) {
    detail::unimplemented();
  }


  /** Set kernel args for an OpenCL kernel which is used through the
      SYCL/OpenCL interoperability interface

      The index value specifies which parameter of the OpenCL kernel is
      being set and the accessor object, which OpenCL buffer or image is
      going to be given as kernel argument.

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
    current_task->schedule(F);
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
    current_task->schedule([=] { detail::parallel_for(global_size, f); }); \
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
#define TRISYCL_ParallelForFunctor_GLOBAL_OFFSET(N)     \
  template <typename KernelName = std::nullptr_t,       \
            typename ParallelForFunctor>                \
  void parallel_for(range<N> global_size,               \
                    id<N> offset,                       \
                    ParallelForFunctor f) {             \
    current_task->schedule([=] {                        \
        detail::parallel_for_global_offset(global_size, \
                                           offset,      \
                                           f); });      \
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
    current_task->schedule([=] { detail::parallel_for(r, f); });
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
    current_task->schedule([=] { detail::parallel_for_workgroup(r, f); });
  }


  /** Kernel invocation method of a kernel defined as pointer to a kernel
      object, described in detail in 3.5.3

      \todo To be implemented
  */
  void single_task(kernel syclKernel) {
    detail::unimplemented();
  }


  /** Kernel invocation method of a kernel defined as pointer to a kernel
      object, for the specified range and given an id or item for indexing
      in the indexing space defined by range, described in detail in 3.5.3

      \todo To be implemented
  */
  template <std::size_t Dimensions = 1>
  void parallel_for(range<Dimensions> numWorkItems,
                    kernel sycl_kernel) {
    detail::unimplemented();
  }


  /** Kernel invocation method of a kernel defined as pointer to a kernel
      object, for the specified nd_range and given an nd_item for indexing
      in the indexing space defined by the nd_range, described in detail
      in 3.5.3

      \todo To be implemented
  */
  template <std::size_t Dimensions = 1>
  void parallel_for(nd_range<Dimensions>, kernel syclKernel) {
    detail::unimplemented();
  }

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

#endif // TRISYCL_SYCL_HANDLER_HPP
