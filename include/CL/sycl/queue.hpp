#ifndef TRISYCL_SYCL_QUEUE_HPP
#define TRISYCL_SYCL_QUEUE_HPP

/** \file The OpenCL SYCL queue

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/context.hpp"
#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device.hpp"
#include "CL/sycl/device_selector.hpp"
#include "CL/sycl/handler.hpp"
#include "CL/sycl/handler_event.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/parallelism.hpp"

namespace cl {
namespace sycl {

class context;
class device_selector;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

namespace info {

using queue_profiling = bool;

/** Queue information descriptors

    From specification C.4

    \todo unsigned int?

    \todo To be implemented
*/
enum class queue : int {
  context,
  device,
  reference_count,
  properties
};

/** Dummy example for get_info() on queue::context that would return a
    context

    \todo Describe all the types
*/
TRISYCL_INFO_PARAM_TRAITS(queue::context, context)

}


/** SYCL queue, similar to the OpenCL queue concept.

    \todo The implementation is quite minimal for now. :-)
*/
class queue {

public:
  /** This constructor creates a SYCL queue from an OpenCL queue

      At construction it does a retain on the queue memory object.

      Retain a reference to the cl_command_queue object. Caller should
      release the passed cl_command_queue object when it is no longer
      needed.

      Return synchronous errors regarding the creation of the queue and
      report asynchronous errors via the async_handler callback function
      in conjunction with the synchronization and throw methods.

      Note that the default case asyncHandler = nullptr is handled by the
      default constructor.

  */
  explicit queue(async_handler asyncHandler) {
    detail::unimplemented();
  }


  /** Creates a queue for the device provided by the device selector

      If no device is selected, an error is reported.

      Return synchronous errors regarding the creation of the queue and
      report asynchronous errors via the async_handler callback
      function if and only if there is an async_handler provided.
  */
  queue(const device_selector &deviceSelector,
        async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


  /** A queue is created for syclDevice

      Return asynchronous errors via the async_handler callback function.
  */
  queue(const device &syclDevice,
        async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  };


  /** This constructor chooses a device based on the provided
      device_selector, which needs to be in the given context.

      If no device is selected, an error is reported.

      Return synchronous errors regarding the creation of the queue.

      If and only if there is an asyncHandler provided, it reports
      asynchronous errors via the async_handler callback function in
      conjunction with the synchronization and throw methods.
  */
  queue(const context &syclContext,
        const device_selector &deviceSelector,
        async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


  /** Creates a command queue using clCreateCommandQueue from a context
      and a device

      Return synchronous errors regarding the creation of the queue.

      If and only if there is an asyncHandler provided, it reports
      asynchronous errors via the async_handler callback function in
      conjunction with the synchronization and throw methods.
  */
  queue(const context &syclContext,
        const device &syclDevice,
        async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


  /** Creates a command queue using clCreateCommandQueue from a context
      and a device

      It enables profiling on the queue if the profilingFlag is set to
      true.

      Return synchronous errors regarding the creation of the queue. If
      and only if there is an asyncHandler provided, it reports
      asynchronous errors via the async_handler callback function in
      conjunction with the synchronization and throw methods.
  */
  queue(const context &syclContext,
        const device &syclDevice,
        info::queue_profiling profilingFlag,
        async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


#ifdef TRISYCL_OPENCL
  /** This constructor creates a SYCL queue from an OpenCL queue

      At construction it does a retain on the queue memory object.

      Return synchronous errors regarding the creation of the queue. If
      and only if there is an async_handler provided, it reports
      asynchronous errors via the async_handler callback function in
      conjunction with the synchronization and throw methods.
  */
  queue(const cl_command_queue &clQueue,
        async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }
#endif


  /// Get the default constructors back.
  queue() = default;


#ifdef TRISYCL_OPENCL
  /** Return the underlying OpenCL command queue after doing a retain

      This memory object is expected to be released by the developer.

      Retain a reference to the returned cl_command_queue object.

      Caller should release it when finished.

      If the queue is a SYCL host queue then a nullptr will be returned.
  */
  cl_command_queue get() const {
    detail::unimplemented();
    return {};
  }
#endif


  /** Return the SYCL queue’s context

      Report errors using SYCL exception classes.
  */
  context get_context() const {
    detail::unimplemented();
    return {};
  }


  /** Return the SYCL device the queue is associated with

      Report errors using SYCL exception classes.
  */
  device get_device() const {
    detail::unimplemented();
    return {};
  }


  /** Return whether the queue is executing on a SYCL host device
  */
  bool is_host() const {
    detail::unimplemented();
    return true;
  }

  /** Performs a blocking wait for the completion all enqueued tasks in
      the queue

      Synchronous errors will be reported through SYCL exceptions.
  */
  void wait() {
    detail::unimplemented();
  }


  /** Perform a blocking wait for the completion all enqueued tasks in the queue

      Synchronous errors will be reported via SYCL exceptions.

      Asynchronous errors will be passed to the async_handler passed to the
      queue on construction.

      If no async_handler was provided then asynchronous exceptions will
      be lost.
  */
  void wait_and_throw() {
    detail::unimplemented();
  }


  /** Checks to see if any asynchronous errors have been produced by the
      queue and if so reports them by passing them to the async_handler
      passed to the queue on construction

      If no async_handler was provided then asynchronous exceptions will
      be lost.
  */
  void throw_asynchronous() {
    detail::unimplemented();
  }


  /// Queries the platform for cl_command_queue info
  template <info::queue param>
  typename info::param_traits<info::queue, param>::type get_info() const  {
    detail::unimplemented();
    return {};
  }


  /** Submit a command group functor to the queue, in order to be
      scheduled for execution on the device

      Use an explicit functor parameter taking a handler& so we can use
      "auto" in submit() lambda parameter.
  */
  handler_event submit(std::function<void(handler &)> cgf) {
    handler command_group_handler;
    cgf(command_group_handler);
    return {};
  }


  /** Submit a command group functor to the queue, in order to be
      scheduled for execution on the device

      On kernel error, this command group functor, then it is scheduled
      for execution on the secondary queue.

      Return a command group functor event, which is corresponds to the
      queue the command group functor is being enqueued on.
  */
  handler_event submit(std::function<void(handler &)> cgf, queue &secondaryQueue) {
    detail::unimplemented();
    // Since it is not implemented, always submit on the main queue
    return submit(cgf);
  }

};

/// @} to end the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_QUEUE_HPP
