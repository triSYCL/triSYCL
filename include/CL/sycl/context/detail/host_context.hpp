#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_HOST_CONTEXT_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_HOST_CONTEXT_HPP

/** \file The OpenCL SYCL host device implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/default_classes.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/detail/singleton.hpp"
#include "CL/sycl/exception.hpp"
#include "CL/sycl/info/param_traits.hpp"
#include "CL/sycl/info/platform.hpp"
#include "CL/sycl/context/detail/context.hpp"

namespace cl {
namespace sycl {
namespace detail {


/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/
    
    class host_context : public detail::context,
			 public detail::singleton<host_context>{

public:

#ifdef TRISYCL_OPENCL
  /** Return the cl_device_id of the underlying OpenCL platform

      This throws an error since there is no OpenCL device associated
      to the host device.
  */
  cl_context get() const override {
    throw non_cl_error("The host device has no OpenCL context");
  }

  boost::compute::context get_boost_compute() const override {
    int i = 1/0;
    throw non_cl_error("The host device has no boost context");
  }
#endif

    bool is_host() const override {
	return true;
    }

    
    cl::sycl::platform get_platform() const override {
	detail::unimplemented();
	return {};
    }

#if 0
    template <info::context Param>
    typename info::param_traits<info::context, Param>::type
    get_info() const override {
	detail::unimplemented();
	return {};
    }
#endif
	
   vector_class<cl::sycl::device>
    get_devices() const override {
	detail::unimplemented();
	return {};
    }
};
    
/// @} to end the execution Doxygen group

}
}
}
#endif // TRISYCL_SYCL_CONTEXT_DETAIL_HOST_CONTEXT_HPP
