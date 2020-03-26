#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_HOST_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_HOST_CONTEXT_HPP

/** \file The OpenCL SYCL host context implementation

    a-doumoulakis at gmail dot com (Anastasios Doumoulakis)

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/default_classes.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/detail/singleton.hpp"
#include "triSYCL/exception.hpp"
#include "triSYCL/info/param_traits.hpp"
#include "triSYCL/info/platform.hpp"
#include "triSYCL/context/detail/context.hpp"

namespace trisycl::detail {


/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/

/** SYCL host context

    \todo The implementation is quite minimal for now. :-)
*/
class host_context : public detail::context,
                     public detail::singleton<host_context> {

public:

#ifdef TRISYCL_OPENCL
  /** Return the underlying \c cl_context of the \c trisycl::context

      This throws an error since there is no OpenCL context associated
      to the host device.
  */
  cl_context get() const override {
    throw non_cl_error("The host context has no OpenCL context");
  }


  /** Return the SYCL platform that the context is initialized for

      This throws an error since there is no \c boost::compute context
      associated to the host device.
  */
  boost::compute::context &get_boost_compute() override {
    throw non_cl_error("The host context has no OpenCL context");
  }


  /** Return the internal OpenCL queue that is associated to the host
      context

      This throws an error since there is no \c
      boost::compute::command_queue associated to the host context.
  */
  boost::compute::command_queue &get_boost_queue() override {
    throw non_cl_error("The host context cannot have an OpenCL queue");
  }
#endif


  /// Return true since the context is a SYCL host context
  bool is_host() const override {
    return true;
  }


  /** Return the platform of the context

      Return synchronous errors via the SYCL exception class.
  */
  trisycl::platform get_platform() const override {
    // Return the host platform
    return {};
  }

#if 0
  /** Query the context for OpenCL \c info::context info

      Return synchronous errors via the SYCL exception class.

      \todo To be implemented
  */
  template <info::context Param>
  typename info::param_traits<info::context, Param>::type
  get_info() const override {
    TRISYCL_UNIMPL;
    return {};
  }
#endif


  /** Returns the set of devices that are part of this context.
      It should only return the host device itself.

      \todo To be implemented
  */
  vector_class<trisycl::device>
  get_devices() const override {
    // Return just the host device
    return { {} };
  }

  /// Return 0 since the context is a SYCL host context
  trisycl::cl_uint get_reference_count() const override {
    return 0;
  }
};

/// @} to end the execution Doxygen group

}
#endif // TRISYCL_SYCL_CONTEXT_DETAIL_HOST_CONTEXT_HPP
