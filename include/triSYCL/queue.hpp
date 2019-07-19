#ifndef TRISYCL_SYCL_QUEUE_HPP
#define TRISYCL_SYCL_QUEUE_HPP

/** \file The OpenCL SYCL queue

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/context.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/default_classes.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/detail/property.hpp"
#include "triSYCL/device.hpp"
#include "triSYCL/device_selector.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/handler.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/info/queue.hpp"
#include "triSYCL/parallelism.hpp"
#include "triSYCL/property_list.hpp"
#include "triSYCL/queue/detail/host_queue.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/queue/detail/opencl_queue.hpp"
#endif

namespace trisycl {

class context;
class device_selector;

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL queue, similar to the OpenCL queue concept.

    \todo The implementation is quite minimal for now. :-)

    \todo All the queue methods should return a queue& instead of void
    to it is possible to chain opoerations
*/
class queue
    /* Use the underlying queue implementation that can be shared in
       the SYCL model */
  : public detail::shared_ptr_implementation<queue, detail::queue>,
    public property_list,
    detail::debug<queue>
{

  // The type encapsulating the implementation
  using implementation_t = typename queue::shared_ptr_implementation;

  /* Allows the comparison operation to sneak in

     Required from Clang++ 3.9 and G++ 6
  */
  friend implementation_t;

public:

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  /** Default constructor for queue from the default queue selector

      Returns errors via the SYCL exception class.
  */
  explicit queue(const property_list &propList = {});

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
  queue(const async_handler &asyncHandler, const property_list &propList = {});

  /** Creates a queue for the device provided by the device selector

      If no device is selected, an error is reported.

      Return synchronous errors regarding the creation of the queue and
      report asynchronous errors via the \c async_handler callback
      function if and only if there is an \c async_handler provided.
  */
  queue(const device_selector &deviceSelector,
        const property_list &propList = {})
    // Just create the queue from the selected device
    : queue { device { deviceSelector }, nullptr, propList } { }

  /** Creates a queue for the device provided by the device selector.

      Return synchronous errors regarding the creation of the queue and
      report asynchronous errors via the \c async_handler callback
      function if and only if there is an \c async_handler provided.
   */
  queue(const device_selector &deviceSelector,
        const async_handler &asyncHandler,
        const property_list &propList = {})
    // Just create the queue from the selected device
    : queue { device { deviceSelector }, asyncHandler, propList } { }

  /** A queue is created for a SYCL device

      Return asynchronous errors via the \c async_handler callback
      function.
  */
  queue(const device &d,
        const async_handler &asyncHandler,
        const property_list &propList = {}) : implementation_t {
#ifdef TRISYCL_OPENCL
    d.is_host()
      ? std::shared_ptr<detail::queue>{ new detail::host_queue }
      : detail::opencl_queue::instance(d)
#else
    new detail::host_queue
#endif
  }, property_list { propList } { }

  /** A queue is created for a SYCL device

  */
  queue(const device &d,
        const property_list &propList = {})
    : queue { d, {}, propList } {}

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
        const async_handler &asyncHandler,
        const property_list &propList = {})
    : property_list { propList } {
    device d = deviceSelector.select_device();
    vector_class<device> ctx_devs = syclContext.get_devices();
    if (std::find(ctx_devs.begin(), ctx_devs.end(), d) == ctx_devs.end())
      throw trisycl::invalid_object_error("Device doesn't belong to context\n");
    implementation =
#ifdef TRISYCL_OPENCL
      d.is_host() ? std::shared_ptr<detail::queue>{ new detail::host_queue }
    : detail::opencl_queue::instance(d);
#else
    std::shared_ptr<detail::queue>{ new detail::host_queue };
#endif
  }

  /** This constructor chooses a device based on the provided
      device_selector, which needs to be in the given context.

      If no device is selected, an error is reported.

      Return synchronous errors regarding the creation of the queue.
  */
  queue(const context &syclContext,
        const device_selector &deviceSelector,
        const property_list &propList = {})
    : queue { syclContext, deviceSelector, {}, propList } {}

#ifdef TRISYCL_OPENCL
  /** This constructor creates a SYCL queue from an OpenCL queue

      At construction it does a retain on the queue memory object.

      Return synchronous errors regarding the creation of the queue. If
      and only if there is an async_handler provided, it reports
      asynchronous errors via the async_handler callback function in
      conjunction with the synchronization and throw methods.
  */
  queue(const cl_command_queue &q, const context &syclContext,
        const async_handler &ah = {})
    : queue { boost::compute::command_queue { q }, ah } {}


  /** Construct a queue instance using a boost::compute::command_queue

      This is a triSYCL extension for boost::compute interoperation.

      Return synchronous errors via the SYCL exception class.

      \todo Deal with handler
  */
  queue(const boost::compute::command_queue &q, async_handler ah = nullptr)
    : implementation_t { detail::opencl_queue::instance(q) }, property_list {} {}
#endif


#ifdef TRISYCL_OPENCL
  /** Return the underlying OpenCL command queue after doing a retain

      This memory object is expected to be released by the developer.

      Retain a reference to the returned cl_command_queue object.

      Caller should release it when finished.

      If the queue is a SYCL host queue then an exception is thrown.
  */
  cl_command_queue get() const {
    return implementation->get();
  }


  /** Return the underlying Boost.Compute command queue if it is an
      OpenCL queue

      This is a triSYCL extension
  */
  boost::compute::command_queue get_boost_compute() const {
    return implementation->get_boost_compute();
  }
#endif


  /** Return the SYCL queue's context

      Report errors using SYCL exception classes.
  */
  context get_context() const {
    return implementation->get_context();
  }


  /** Return the SYCL device the queue is associated with

      Report errors using SYCL exception classes.
  */
  device get_device() const {
    return implementation->get_device();
  }


  /// Return whether the queue is executing on a SYCL host device
  bool is_host() const {
    return implementation->is_host();
  }


  /** Performs a blocking wait for the completion all enqueued tasks in
      the queue

      Synchronous errors will be reported through SYCL exceptions.
  */
  void wait() {
    implementation->wait_for_kernel_execution();
  }


  /** Perform a blocking wait for the completion all enqueued tasks in the queue

      Synchronous errors will be reported via SYCL exceptions.

      Asynchronous errors will be passed to the async_handler passed to the
      queue on construction.

      If no async_handler was provided then asynchronous exceptions will
      be lost.
  */
  void wait_and_throw() {
    // \todo Implement the throw part of wait_and_throw
    wait();
    TRISYCL_UNIMPL;
  }


  /** Checks to see if any asynchronous errors have been produced by the
      queue and if so reports them by passing them to the async_handler
      passed to the queue on construction

      If no async_handler was provided then asynchronous exceptions will
      be lost.
  */
  void throw_asynchronous() {
    TRISYCL_UNIMPL;
  }


  /// Queries the platform for cl_command_queue info
  template <info::queue param>
  inline auto get_info() const;

  /** Submit a command group functor to the queue, in order to be
      scheduled for execution on the device

      Use an explicit functor parameter taking a handler& so we can use
      "auto" in submit() lambda parameter.

      \todo Add in the spec an implicit conversion of event to
      queue& so it is possible to chain operations on the queue
  */
  template <typename Handler_Functor>
  event submit(Handler_Functor cgf) {
    handler command_group_handler { implementation };
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
  template <typename Handler_Functor>
  event submit(Handler_Functor cgf, queue &secondaryQueue) {
    TRISYCL_UNIMPL;
    // Since it is not implemented, always submit on the main queue
    return submit(cgf);
  }

  /** Check if the queue was constructed with the specified
      property.
  */
  template <typename propertyT>
  bool has_property() const {
    return property_list::has_property<propertyT>();
  }

  /** Return a copy of the property that the queue was
      constructed with.
  */
  template <typename propertyT>
  propertyT get_property() const {
    return property_list::get_property<propertyT>();
  }
};

template<>
inline auto queue::get_info<info::queue::device>() const {
  return implementation->get_device();
}

template<>
inline auto queue::get_info<info::queue::context>() const {
  return implementation->get_context();
}

template<>
inline auto queue::get_info<info::queue::reference_count>() const {
  return trisycl::cl_uint {0};
}
/// @} to end the execution Doxygen group

}

/* Inject a custom specialization of std::hash to have the buffer
   usable into an unordered associative container

   \todo Add this to the spec
*/
namespace std {

template <> struct hash<trisycl::queue> {

  auto operator()(const trisycl::queue &q) const {
    // Forward the hashing to the implementation
    return q.hash();
  }

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_QUEUE_HPP
