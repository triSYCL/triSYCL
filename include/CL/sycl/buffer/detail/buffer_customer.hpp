#ifndef TRISYCL_SYCL_BUFFER_CUSTOMER_HPP
#define TRISYCL_SYCL_BUFFER_CUSTOMER_HPP

/** \file The concept of buffer_customer behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "CL/sycl/detail/debug.hpp"

namespace cl {
namespace sycl {
namespace detail {

struct buffer_base;
/// \todo is it needed?
struct task;

/** Keep track of the tasks waiting for the availability of a buffer
    generation, either to read it or to write it

    When we write into a buffer, we generate a new version of it (think
    "SSA")
*/
class buffer_customer : public detail::debug<buffer_customer> {
  /// The considered buffer
  /// \todo Do we need to keep it?
  buffer_base &buf;
  /// At some point use lock free list for this inside buffer_base
  std::shared_ptr<buffer_customer> next_generation;
  /// \todo Needed?
  bool write_access;
  // State when the buffer generation is ready to be used
  bool ready_to_use;
  // To signal when it is ready
  std::condition_variable ready_cv;
  /// To protect the access to the condition variable
  std::mutex ready_mutex;
  /// Count the number of accelerator-side usage of this buffer generation
  std::atomic<unsigned int> user_number;
  /** To signal when the buffer generation is no longer used from the
      accelerator side and can be used for example through a host
      accessor */
  std::condition_variable released_cv;
  /// To protect the access to the condition variable
  std::mutex released_mutex;

public:

  buffer_customer(buffer_base &buf, bool is_write_access)
    : buf { buf },  write_access { is_write_access },
      ready_to_use { false }, user_number { 0 } {
  }


  /** Set the next generation of the buffer after this

      \todo Refactor this with an lock-free list?
  */
  void set_next_generation(std::shared_ptr<buffer_customer> bc) {
    next_generation = bc;
  }


  /// Add a new task as a customer of the buffer generation
  void add(std::shared_ptr<task> task, bool is_write_access) {
    write_access = is_write_access;
    user_number++;
    TRISYCL_DUMP_T("buffer_customer::add() now user_number = " << user_number);
  }


  /// Wait for the buffer generation to be ready to use by a kernel task
  void wait() {
    {
      std::unique_lock<std::mutex> ul { ready_mutex };
      ready_cv.wait(ul, [&] { return ready_to_use; });
    }
  }


  /// Release the buffer generation usage by a kernel task
  void release() {
    user_number--;
    TRISYCL_DUMP_T("buffer_customer::release() now user_number = "
                   << user_number);
    if (user_number == 0) {
      /* If there is no task using this generation of the buffer, first
         notify the host accessors waiting for it, if any */
      released_cv.notify_all();

      /* And then make the next generation ready if any. Note that if the
         SYCL program is race condition-free, there should be no host
         accessor waiting for a generation which is not the last one...

         \todo: add some SYCL semantics runtime verification
      */
      if (next_generation)
        next_generation->notify_ready();
    }
    // \todo Can we have UserNumber increasing again?
  }


  /** Wait for the release of the buffer generation before the host can
      use it
  */
  void wait_released() {
    TRISYCL_DUMP_T("buffer_customer::wait_released() user_number = "
                   << user_number);
    {
      std::unique_lock<std::mutex> ul { released_mutex };
      released_cv.wait(ul, [&] { return user_number == 0; });
    }
  }


  /// Notify the customer tasks this buffer generation is ready to use
  void notify_ready() {
    {
      std::unique_lock<std::mutex> ul { ready_mutex };
      // \todo This lock can be avoided if ready_to_use is atomic
      ready_to_use = true;
    }
    TRISYCL_DUMP_T("buffer_customer::notify_ready()");
    ready_cv.notify_all();
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

#endif // TRISYCL_SYCL_BUFFER_CUSTOMER_HPP
