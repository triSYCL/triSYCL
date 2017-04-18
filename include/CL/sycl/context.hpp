#ifndef TRISYCL_SYCL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_HPP

/** \file The OpenCL SYCL context

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/device.hpp"
#include "CL/sycl/device_selector.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/platform.hpp"

#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/info/context.hpp"
#include "CL/sycl/context/detail/host_context.hpp"
#ifdef TRISYCL_OPENCL
#include "CL/sycl/context/detail/opencl_context.hpp"
#endif


namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL context

    The context class encapsulates an OpenCL context, which is implicitly
    created and the lifetime of the context instance defines the lifetime
    of the underlying OpenCL context instance.

    On destruction clReleaseContext is called.

    The default context is the SYCL host context containing only the SYCL
    host device.

    \todo The implementation is quite minimal for now.
*/
class context

  /* Use the underlying context implementation that can be shared in the
     SYCL model */
  : public detail::shared_ptr_implementation<context, detail::context> {

  // The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<context, detail::context>;

public:

    // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  /** Constructs a context object for SYCL host using an async_handler for
      handling asynchronous errors

      Note that the default case asyncHandler = nullptr is handled by the
      default constructor.
  */
  explicit context(async_handler asyncHandler) {
    detail::unimplemented();
  }


#ifdef TRISYCL_OPENCL
  /* Context constructor, where the underlying OpenCL context is given as
     a parameter

     The constructor executes a retain on the cl_context.

     Return synchronous errors via the SYCL exception class and
     asynchronous errors are handled via the async_handler, if provided.
  */
  context(cl_context clContext, async_handler asyncHandler = nullptr)
    : context { boost::compute::context { clContext }, asyncHandler } {}

  context(const boost::compute::context &c,
          async_handler asyncHandler = nullptr)
    : implementation_t { detail::opencl_context::instance(c) } {}
#endif

  /** Constructs a context object using a device_selector object

      The context is constructed with a single device retrieved from the
      device_selector object provided.

      Return synchronous errors via the SYCL exception class and
      asynchronous errors are handled via the async_handler, if provided.
  */
  context(const device_selector &deviceSelector,
          info::gl_context_interop interopFlag,
          async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


  /** Constructs a context object using a device object

      Return synchronous errors via the SYCL exception class and
      asynchronous errors are handled via the async_handler, if provided.
  */
  context(const device &dev,
          info::gl_context_interop interopFlag,
          async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


  /** Constructs a context object using a platform object

      Return synchronous errors via the SYCL exception class and
      asynchronous errors are handled via the async_handler, if provided.
  */
  context(const platform &plt,
          info::gl_context_interop interopFlag,
          async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }


  /* Constructs a context object using a vector_class of device objects

     Return synchronous errors via the SYCL exception class and
     asynchronous errors are handled via the async_handler, if provided.

     \todo Update the specification to replace vector by collection
     concept.
  */
  context(const vector_class<device> &deviceList,
          info::gl_context_interop interopFlag,
          async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }

  /** Default constructor that chooses the context according the
      heuristics of the default selector

      Return synchronous errors via the SYCL exception class.

      Get the default constructors back.
  */
  context() : implementation_t { detail::host_context::instance() } {}


#ifdef TRISYCL_OPENCL
  /* Returns the underlying cl_context object, after retaining the cl_context.

     Retains a reference to the returned cl_context object.

     Caller should release it when finished.
  */
  cl_context get() const {
    return implementation->get();
  }


  boost::compute::context &get_boost_compute() {
    return implementation->get_boost_compute();
  }

  boost::compute::command_queue &get_boost_queue() {
    return implementation->get_boost_queue();
  }
#endif


  /// Specifies whether the context is in SYCL Host Execution Mode.
  bool is_host() const {
    return implementation->is_host();
  }


  /** Returns the SYCL platform that the context is initialized for

      \todo To be implemented
  */
  platform get_platform();


  /** Returns the set of devices that are part of this context

      \todo To be implemented
  */
  vector_class<device> get_devices() const {
    detail::unimplemented();
    return {};
  }


  /** Queries OpenCL information for the under-lying cl context

      \todo To be implemented
  */
  template <info::context Param>
  typename info::param_traits<info::context, Param>::type get_info() const {
    detail::unimplemented();
    return {};
  }

};

/// @} to end the execution Doxygen group

}
}

namespace std {

  template <> struct hash<cl::sycl::context> {
    auto operator()(const cl::sycl::context &c) const {
    return c.hash();
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

#endif // TRISYCL_SYCL_CONTEXT_HPP
