#ifndef TRISYCL_SYCL_EVENT_DETAIL_EVENT_HPP
#define TRISYCL_SYCL_EVENT_DETAIL_EVENT_HPP

/** \file The OpenCL SYCL abstract event

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl::detail {

struct event : detail::debug<detail::event> {

public:
#ifdef TRISYCL_OPENCL
  /// Return the cl_event of the underlying OpenCL event
  virtual cl_event get() const = 0;

  /// Return the underlying Boost.Compute event, if any
  virtual const boost::compute::event &get_boost_compute() const = 0;
#endif

  virtual bool is_host() const = 0;

  virtual info::event_command_status get_command_execution_status() const = 0;

  virtual cl_ulong get_profiling_info(info::event_profiling param) const = 0;

  virtual cl_uint get_reference_count() const = 0;

  virtual void wait() const = 0;

  virtual ~event() {}
};

}

#endif
