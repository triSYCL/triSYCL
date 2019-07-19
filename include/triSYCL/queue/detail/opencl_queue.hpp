#ifndef TRISYCL_SYCL_QUEUE_DETAIL_OPENCL_QUEUE_HPP
#define TRISYCL_SYCL_QUEUE_DETAIL_OPENCL_QUEUE_HPP

/** \file Some implementation details of the OpenCL queue

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/context.hpp"
#include "triSYCL/detail/cache.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/device.hpp"
#include "triSYCL/queue/detail/queue.hpp"

namespace trisycl::detail {

/// Some implementation details about the SYCL queue
class opencl_queue : public detail::queue,
                     detail::debug<opencl_queue> {
  /// Use the Boost Compute abstraction of the OpenCL command queue
  boost::compute::command_queue q;

  /** A cache to always return the same alive queue for a given OpenCL
      command queue

      C++11 guaranties the static construction is thread-safe
  */
  static detail::cache<cl_command_queue, detail::opencl_queue> cache;

  /// Return the cl_command_queue of the underlying OpenCL queue
  cl_command_queue get() const override {
    return q.get();
  }


  /// Return the underlying Boost.Compute command queue
  boost::compute::command_queue &get_boost_compute() override {
    return q;
  }


  /// Return the SYCL context associated to the queue
  trisycl::context get_context() const override {
    return q.get_context();
  }


  /// Return the SYCL device associated to the queue
  trisycl::device get_device() const override {
    return q.get_device();
  }


  /// Claim proudly that an OpenCL queue cannot be the SYCL host queue
  bool is_host() const override {
    return false;
  }

private:

  /// Only the instance factory can built it
  opencl_queue(const boost::compute::command_queue &q) : q { q } {}

public:

  /// Get a singleton instance of the opencl_queue
  static std::shared_ptr<opencl_queue>
  instance(const boost::compute::command_queue &q) {
    return cache.get_or_register(q.get(),
                                 [&] { return new opencl_queue { q }; });
  }


  /** Create a new queue associated to this device

      \todo Check with SYCL committee what is the expected behaviour
      here about the context. Is this a new context everytime, or
      always the same for a given device?
  */
  static std::shared_ptr<detail::queue>
  instance(const trisycl::device &d) {
    return instance (boost::compute::command_queue {
        // For now, create a new context every time
        boost::compute::context { d.get_boost_compute() },
        d.get_boost_compute()
          });
  }


  /// Unregister from the cache on destruction
  ~opencl_queue() override {
    cache.remove(q.get());
  }

};

/* Allocate the cache here but since this is a pure-header library,
   use a weak symbol so that only one remains when SYCL headers are
   used in different compilation units of a program
*/
TRISYCL_WEAK_ATTRIB_PREFIX
detail::cache<cl_command_queue, detail::opencl_queue> opencl_queue::cache
TRISYCL_WEAK_ATTRIB_SUFFIX;

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_QUEUE_DETAIL_OPENCL_QUEUE_HPP
