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
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/platform.hpp"

namespace cl {
namespace sycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

namespace info {

using gl_context_interop = bool;

/** Context information descriptors

    \todo Should be unsigned int to be consistent with others?
*/
enum class context : int {
  reference_count,
  num_devices,
  gl_interop
};


/** Query the return type for get_info() on context stuff

    \todo To be implemented
*/
TRISYCL_INFO_PARAM_TRAITS_ANY_T(info::context, void)

}


/** SYCL context

    The context class encapsulates an OpenCL context, which is implicitly
    created and the lifetime of the context instance defines the lifetime
    of the underlying OpenCL context instance.

    On destruction clReleaseContext is called.

    The default context is the SYCL host context containing only the SYCL
    host device.

    \todo The implementation is quite minimal for now.
*/
class context {

public:

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
  context(cl_context clContext, async_handler asyncHandler = nullptr) {
    detail::unimplemented();
  }
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
  context() = default;


#ifdef TRISYCL_OPENCL
  /* Returns the underlying cl_context object, after retaining the cl_context.

     Retains a reference to the returned cl_context object.

     Caller should release it when finished.
  */
  cl_context get() const {
    detail::unimplemented();
    return {};
  }
#endif


  /// Specifies whether the context is in SYCL Host Execution Mode.
  bool is_host() const {
    return true;
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

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_CONTEXT_HPP
