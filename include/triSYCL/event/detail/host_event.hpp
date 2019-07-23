#ifndef TRISYCL_SYCL_EVENT_DETAIL_HOST_EVENT_HPP
#define TRISYCL_SYCL_EVENT_DETAIL_HOST_EVENT_HPP

/** \file The OpenCL triSYCL host event implementation

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/singleton.hpp"

namespace trisycl::detail {

class host_event : public detail::event,
                   public detail::singleton<host_event> {

public:
#ifdef TRISYCL_OPENCL
  cl_event get() const override {
    throw non_cl_error("The host event has no OpenCL event");
  }

  boost::compute::event &get_boost_compute() const override {
    throw
      non_cl_error("The host device has no underlying Boost Compute event");
  }
#endif

  bool is_host() const override {
    return true;
  }

  cl_uint get_reference_count() const override {
    return 0;
  }

  info::event_command_status get_command_execution_status() const override {
    return info::event_command_status::submitted;
  }

  cl_ulong get_profiling_info(info::event_profiling param) const override {
    return 0;
  }

  void wait() const override {
  }
};

}

#endif // TRISYCL_SYCL_EVENT_DETAIL_HOST_EVENT_HPP
