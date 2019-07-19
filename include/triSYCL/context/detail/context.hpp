#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_CONTEXT_HPP

/** \file The OpenCL SYCL abstract context

    a-doumoulakis at gmail dot com (Anastasios Doumoulakis)

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/device.hpp"
#include "triSYCL/platform.hpp"
#include "triSYCL/detail/default_classes.hpp"

namespace trisycl::detail {

  /** \addtogroup execution Platforms, contexts, devices and queues
      @{
  */

class context {
public:

#ifdef TRISYCL_OPENCL
  /// Return the underlying \c cl_context of the \c trisycl::context
  virtual cl_context get() const = 0;

  /** Return the underlying \c boost::compute::context
      of the \c trisycl::context
  */
  virtual boost::compute::context &get_boost_compute() = 0;

  /** Return the underlying \c boost::compute::command_queue associated
      with the context
  */
  virtual boost::compute::command_queue &get_boost_queue() = 0;
#endif

  /// Returns true is the context is a SYCL host context
  virtual bool is_host() const = 0;

  /// Returns the SYCL platform that the context is initialized for
  virtual trisycl::platform get_platform() const = 0;

  /** \todo virtual cannot be templated
      template <info::context Param>
      typename info::param_traits<info::context, Param>::type
      get_info() const = 0;
  */

  /// Returns the set of devices that are part of this context.
  virtual vector_class<trisycl::device> get_devices() const = 0;

  /// Returns the reference count of underlying CL context, or 0 on host.
  virtual trisycl::cl_uint get_reference_count() const = 0;

  /// Virtual to call the real destructor
  virtual ~context() {}

};

/// @} to end the execution Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DEVICE_DETAIL_DEVICE_HPP
