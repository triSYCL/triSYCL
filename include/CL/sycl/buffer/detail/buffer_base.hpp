#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP

/** \file The buffer_base behind the buffers

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "CL/sycl/access.hpp"

namespace cl {
namespace sycl {

class handler;

namespace detail {

struct task;
struct buffer_base;
inline static void add_buffer_to_task(handler *command_group_handler,
                                      detail::buffer_base *b,
                                      bool is_write_mode);

/** Factorize some template independent buffer aspects in a base class
 */
struct buffer_base {
  /// If the data are read-only, store the information for later optimization.
  /// \todo Replace this by a static read-only type for the buffer
  bool read_only;

  //// Keep track of the number of kernel accessors using this buffer
  std::atomic<size_t> number_of_users;

  /// Store the latest task to produce this buffer
  std::atomic<detail::task *> latest_producer {};

  /// To signal when this buffer ready
  std::condition_variable ready;
  /// To protect the access to the condition variable
  std::mutex ready_mutex;


  /// Create a buffer base
  buffer_base(bool read_only) : read_only { read_only },
                                number_of_users { 0 } {}


  /// The destructor wait for not being used anymore
  ~buffer_base() {
    wait();
  }


  /// Wait for this buffer to be ready, which is no longer in use
  void wait() {
    std::unique_lock<std::mutex> ul { ready_mutex };
    ready.wait(ul, [&] {
        // When there is no producer for this buffer, we are ready to use it
        return number_of_users == 0;
      });
  }


  /// Mark this buffer in use by a task
  void use() {
    // Increment the use count
    ++number_of_users;
  }


  /// A task has released the buffer
  void release() {
    if (--number_of_users == 0)
      // Notify the host consumers or the buffer destructor that it is ready
      ready.notify_all();
  }


  /// Return the latest producer for the buffer
  detail::task *get_latest_producer() {
    /// No lock explicitly required since it is an atomic type
    return latest_producer;
  }


  /** Return the latest producer for the buffer and set another
      future producer
  */
  detail::task *
  set_latest_producer(detail::task *newer_latest_producer) {
    return latest_producer.exchange(newer_latest_producer);
  }


  /// Add a buffer to the task running the command group
  void add_to_task(handler *command_group_handler, bool is_write_mode) {
    add_buffer_to_task(command_group_handler, this, is_write_mode);
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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP
