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

#include "CL/sycl/detail/debug.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** Some implementation details about the SYCL queue
 */
struct queue {
  /// Track the number of kernels still running to wait for their completion
  std::atomic<size_t> running_kernels;

  /// To signal when all the kernels have completed
  std::condition_variable finished;
  /// To protect the access to the condition variable
  std::mutex finished_mutex;


  /// Initialize the queue with 0 running kernel
  queue() {
    running_kernels = 0;
  }


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
    if (--running_kernels == 0) {
      /* It was the last kernel running, so signal the queue just in
         case it was working for it for completion */
      finished.notify_one();
    }
  }


  /// Wait for all kernel completion before the queue destruction
  ~queue() {
    wait_for_kernel_execution();
  }

};

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_QUEUE_DETAIL_QUEUE_HPP
