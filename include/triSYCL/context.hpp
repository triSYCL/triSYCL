#ifndef TRISYCL_SYCL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_HPP

/** \file The OpenCL SYCL context

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "triSYCL/context/detail/host_context.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/context/detail/opencl_context.hpp"
#endif

#include "triSYCL/detail/default_classes.hpp"
#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/device.hpp"
#include "triSYCL/device_selector.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/info/context.hpp"
#include "triSYCL/platform.hpp"

namespace trisycl {

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

  context(vector_class<device> devList, async_handler asyncHandler,
	  const device_selector &deviceSelector) {
    if (devList.empty())
      devList = {deviceSelector.select_device()};
    if (devList[0].is_host())
      implementation = detail::host_context::instance();
#ifdef TRISYCL_OPENCL
    else
      implementation = detail::opencl_context::instance(
        boost::compute::context { devList[0].get_boost_compute () }
      );
#endif
  }

public:

    // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  /** Constructs a context object for SYCL host using an async_handler for
      handling asynchronous errors

      Note that the default case asyncHandler = nullptr is handled by the
      default constructor.
  */
  explicit context(async_handler asyncHandler)
   : context { device{}, asyncHandler } {}

#ifdef TRISYCL_OPENCL
  /** Make a SYCL context from an OpenCL context

      The constructor executes a retain on the \c cl_context.

      Return synchronous errors via the SYCL exception class and
      asynchronous errors are handled via the \c async_handler, if
      provided.
  */
  context(cl_context clContext, async_handler asyncHandler = nullptr)
    : context { boost::compute::context { clContext }, asyncHandler } {}


  /// Build a SYCL context from a Boost.Compute context
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
  context(const device_selector &deviceSelector, async_handler asyncHandler = nullptr)
    : context (vector_class<device>{}, asyncHandler, deviceSelector) {}

  /** Constructs a context object using a device object

      Return synchronous errors via the SYCL exception class and
      asynchronous errors are handled via the async_handler, if provided.
  */
  context(const device &dev, async_handler asyncHandler = nullptr)
    : context { vector_class<device>{dev}, asyncHandler } {}

  /** Constructs a context object using a platform object

      Return synchronous errors via the SYCL exception class and
      asynchronous errors are handled via the async_handler, if provided.
  */
  context(const platform &plt, async_handler asyncHandler = nullptr)
    : context { plt.get_devices(), asyncHandler } {}

  /* Constructs a context object using a vector_class of device objects

     Return synchronous errors via the SYCL exception class and
     asynchronous errors are handled via the async_handler, if provided.

     \todo Update the specification to replace vector by collection
     concept.
  */
  context(const vector_class<device> &deviceList,
          async_handler asyncHandler = nullptr);

  /** Default constructor that chooses the context according the
      heuristics of the default selector

      Return synchronous errors via the SYCL exception class.

      Get the default constructors back.
  */
  context() : context { async_handler {} } {}

#ifdef TRISYCL_OPENCL
  /** Return the underlying \c cl_context object, after retaining
      the \c cl_context.

     Retains a reference to the returned \c cl_context object.

     Caller should release it when finished.
  */
  cl_context get() const {
    return implementation->get();
  }

  /** Return the underlying \c boost::compute::context
      of the \c trisycl::context
  */
  boost::compute::context &get_boost_compute() const {
    return implementation->get_boost_compute();
  }

  /** Return the internal queue that is associated to the context and
      used by triSYCL to move data between some different contests for
      example
  */
  boost::compute::command_queue &get_boost_queue() const {
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
  platform get_platform() const {
    return implementation->get_platform();
  };


  /** Returns the set of devices that are part of this context

      \todo To be implemented
  */
  vector_class<device> get_devices() const {
    return implementation->get_devices();
  }


  /** Queries OpenCL information for the under-lying cl context

      \todo To be implemented
  */
  template <info::context param>
  inline auto get_info() const;

};

template<>
inline auto context::get_info<info::context::reference_count>() const {
  return implementation->get_reference_count();
}

template<>
inline auto context::get_info<info::context::platform>() const {
  return get_platform();
}

template<>
inline auto context::get_info<info::context::devices>() const {
  return get_devices();
}


/// @} to end the execution Doxygen group

}

namespace std {

  template <> struct hash<trisycl::context> {
    auto operator()(const trisycl::context &c) const {
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
