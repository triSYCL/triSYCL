#ifndef TRISYCL_SYCL_EVENT_DETAIL_OPENCL_EVENT_HPP
#define TRISYCL_SYCL_EVENT_DETAIL_OPENCL_EVENT_HPP

/** \file The triSYCL OpenCL event implementation

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


namespace trisycl::detail {

class opencl_event : public detail::event {
  boost::compute::event e;

  /** A cache to always return the same alive event for a given OpenCL
      event

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_event, detail::opencl_event> cache;
public:
  cl_event get() const override {
    return e.get();
  }

  const boost::compute::event &get_boost_compute() const override {
    return e;
  }

  bool is_host() const override {
    return false;
  }

  info::event_command_status get_command_execution_status() const override {
    return static_cast<info::event_command_status>(e.status());
  }

  cl_ulong get_profiling_info(info::event_profiling param) const override {
    return e.get_profiling_info<cl_ulong>(static_cast<cl_profiling_info>(param));
  }

  trisycl::cl_uint get_reference_count() const override {
    return e.get_info<CL_EVENT_REFERENCE_COUNT>();
  }

  void wait() const override {
    e.wait();
  }

  /// Get a singleton instance of the \c opencl_event
  static std::shared_ptr<opencl_event>
  instance(const boost::compute::event &e) {
    return cache.get_or_register(e.get(),
                                 [&] { return new opencl_event { e }; });
  }
private:
  /// Only the instance factory can build it
  opencl_event(const boost::compute::event &e) :
    e { e } {}

public:
  ~opencl_event() override {
    cache.remove(e.get());
  }
};

}

#endif // TRISYCL_SYCL_EVENT_DETAIL_OPENCL_EVENT_HPP
