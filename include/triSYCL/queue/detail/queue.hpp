#ifndef TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_HPP
#define TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_HPP

/** \file Some implementation details of queue.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#include <condition_variable>
#include <mutex>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/context.hpp"
#include "triSYCL/device.hpp"
#include "triSYCL/detail/debug.hpp"

namespace trisycl::detail {

/** Some implementation details about the SYCL queue
 */
struct queue : detail::debug<detail::queue> {
  /// Track the number of kernels still running to wait for their completion
  std::atomic<size_t> running_kernels;

  /// To signal when all the kernels have completed
  std::condition_variable finished;
  /// To protect the access to the condition variable
  std::mutex finished_mutex;


  /// Initialize the queue with 0 running kernel
  queue() : running_kernels { 0 } {}

  /// Wait for all kernel completion
  void wait_for_kernel_execution() {
    TRISYCL_DUMP_T("Queue waiting for kernel completion");
    std::unique_lock<std::mutex> ul { finished_mutex };
    finished.wait(ul, [&] {
        // When there is no kernel running in this queue, we are ready to go
        return running_kernels == 0;
      });
  }


  /// Signal that a new kernel started on this queue
  void kernel_start() {
    TRISYCL_DUMP_T("A kernel has been added to the queue");
    // One more kernel
    ++running_kernels;
  }


  /// Signal that a new kernel finished on this queue
  void kernel_end() {
    TRISYCL_DUMP_T("A kernel of the queue ended");
    std::unique_lock<std::mutex> ul { finished_mutex };
    if (--running_kernels == 0) {
      // Micro-optimization: unlock before the notification
      // https://en.cppreference.com/w/cpp/thread/condition_variable/notify_all
      ul.unlock();
      /* It was the last kernel running, so signal the queue just in
         case it was working for it for completion

         In some cases several threads might want to wait for the
         same queue, because of this \c notify_one is not be enough
         and a \c notify_all is needed
      */
      finished.notify_all();
    }
  }


#ifdef TRISYCL_OPENCL
  /** Return the underlying OpenCL command queue after doing a retain

      This memory object is expected to be released by the developer.

      Retain a reference to the returned cl_command_queue object.

      Caller should release it when finished.

      If the queue is a SYCL host queue then an exception is thrown.
  */
  virtual cl_command_queue get() const = 0;

  /// Return the underlying Boost.Compute command queue
  virtual boost::compute::command_queue &get_boost_compute() = 0;
#endif


  /** Return the SYCL queue's context

      Report errors using SYCL exception classes.
  */
  virtual trisycl::context get_context() const = 0;


  /** Return the SYCL device the queue is associated with

      Report errors using SYCL exception classes.
  */
  virtual trisycl::device get_device() const = 0;


  /// Return whether the queue is executing on a SYCL host device
  virtual bool is_host() const = 0;


  /// Wait for all kernel completion before the queue destruction
  /// \todo Update according spec since queue destruction is non blocking
  virtual ~queue() {
    wait_for_kernel_execution();
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

#endif // TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_HPP
