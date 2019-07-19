#ifndef TRISYCL_SYCL_EVENT_HPP
#define TRISYCL_SYCL_EVENT_HPP

/** \file The event class

    Ronan at keryell dot FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#include "triSYCL/info/event.hpp"
#include "triSYCL/event/detail/event.hpp"
#include "triSYCL/event/detail/host_event.hpp"
#ifdef TRISYCL_OPENCL
#include "triSYCL/event/detail/opencl_event.hpp"
#endif

namespace trisycl {

/// SYCL event.
class event : public detail::shared_ptr_implementation<event, detail::event> {

  // The type encapsulating the implementation
  using implementation_t = typename event::shared_ptr_implementation;

  friend implementation_t;
public:

  event() : implementation_t { detail::host_event::instance() } {}

#ifdef TRISYCL_OPENCL
  /** Construct an event class using the clEvent from OpenCL.

      This is a triSYCL extension for boost::compute interoperation.
  */
  event(cl_event clEvent, const context& syclContext)
    : event { boost::compute::event { clEvent } } {}

  /** Construct an event class using the boost::compute::event.

      This is a triSYCL extension for boost::compute interoperation.
  */
  event(const boost::compute::event &e)
    : implementation_t { detail::opencl_event::instance(e) } {}
#endif

  /// Return true if the event is a SYCL host event
  bool is_host() const { return implementation->is_host(); }

#ifdef TRISYCL_OPENCL
  /// Return the cl_event from the underlying OpenCL event.
  cl_event get() {
    return implementation->get();
  }

  /** Return the underlying boost compute event

      This is a triSYCL extension
  */
  boost::compute::event get_boost_compute() const {
    return implementation->get_boost_compute();
  }
#endif

  vector_class<event> get_wait_list() {
    TRISYCL_UNIMPL;
    return {};
  }

  /** Wait for the event and the command associated with it to complete.

   */
  void wait() {
    implementation->wait();
  }

  static void wait(const vector_class<event> &eventList) {
    TRISYCL_UNIMPL;
  }

  void wait_and_throw() {
    TRISYCL_UNIMPL;
  }

  static void wait_and_throw(const vector_class<event> &eventList) {
    TRISYCL_UNIMPL;
  }

  /// Query the event for information
  template <info::event param>
  inline auto get_info() const;

  /// Query the event for event profiling
  template <info::event_profiling param>
  inline auto get_profiling_info() const;

};

template<>
inline auto event::get_info<info::event::command_execution_status>() const {
  return implementation->get_command_execution_status();
}

template<>
inline auto event::get_info<info::event::reference_count>() const {
  return implementation->get_reference_count();
}

template<>
inline auto event::get_profiling_info<info::event_profiling::command_submit>() const {
  return implementation->get_profiling_info(info::event_profiling::command_submit);
}

template<>
inline auto event::get_profiling_info<info::event_profiling::command_start>() const {
  return implementation->get_profiling_info(info::event_profiling::command_start);
}

template<>
inline auto event::get_profiling_info<info::event_profiling::command_end>() const {
  return implementation->get_profiling_info(info::event_profiling::command_end);
}

}


namespace std {

template <> struct hash<trisycl::event> {

  auto operator()(const trisycl::event &e) const {
    // Forward the hashing to the implementation
    return e.hash();
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

#endif // TRISYCL_SYCL_EVENT_HPP
