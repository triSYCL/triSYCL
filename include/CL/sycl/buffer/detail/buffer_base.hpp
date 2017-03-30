#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP

/** \file The buffer_base behind the buffers, independent of the data
    type

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <utility>
#include <unordered_map>

// \todo Use C++17 optional when it is mainstream
#include <boost/optional.hpp>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#include "CL/sycl/context.hpp"
#endif

#include "CL/sycl/command_group/detail/task.hpp"

namespace cl {
namespace sycl {

class handler;

namespace detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

struct task;
struct buffer_base;
inline static std::shared_ptr<detail::task>
add_buffer_to_task(handler *command_group_handler,
                   std::shared_ptr<detail::buffer_base> b,
                   bool is_write_mode);



/** Factorize some template independent buffer aspects in a base class
 */
struct buffer_base : public std::enable_shared_from_this<buffer_base> {

  //// Keep track of the number of kernel accessors using this buffer
  std::atomic<size_t> number_of_users;

  /// Track the latest task to produce this buffer
  std::weak_ptr<detail::task> latest_producer;
  /// To protect the access to latest_producer
  std::mutex latest_producer_mutex;

  /// To signal when this buffer ready
  std::condition_variable ready;
  /// To protect the access to the condition variable
  std::mutex ready_mutex;

  /** If the SYCL user buffer destructor is blocking, use this to
      block until this buffer implementation is destroyed.

      Use a void promise since there is no value to send, only
      waiting */
  boost::optional<std::promise<void>> notify_buffer_destructor;

#ifdef TRISYCL_OPENCL
  /** Buffer side cache that keeps the boost::compute::buffer (and the
      underlying cl_buffer) so that if the buffer already exists inside
      the same context it is not recreated.
   */
  std::unordered_map<cl::sycl::context, boost::compute::buffer> buffer_cache;
#endif

  /// Create a buffer base
  buffer_base() : number_of_users { 0 } {}


  /// The destructor wait for not being used anymore
  ~buffer_base() {
    wait();
    // If there is the last SYCL user buffer waiting, notify it
    if (notify_buffer_destructor)
      notify_buffer_destructor->set_value();
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
  std::shared_ptr<detail::task> get_latest_producer() {
    std::lock_guard<std::mutex> lg { latest_producer_mutex };
    // Return the valid shared_ptr to the task, if any
    return latest_producer.lock();
  }


  /** Return the latest producer for the buffer and set another
      future producer
  */
  std::shared_ptr<detail::task>
  set_latest_producer(std::weak_ptr<detail::task> newer_latest_producer) {
    std::lock_guard<std::mutex> lg { latest_producer_mutex };
    using std::swap;

    swap(newer_latest_producer, latest_producer);
    // Return the valid shared_ptr to the previous producing task, if any
    return newer_latest_producer.lock();
  }


  /// Add a buffer to the task running the command group
  std::shared_ptr<detail::task>
  add_to_task(handler *command_group_handler, bool is_write_mode) {
    return add_buffer_to_task(command_group_handler,
                              shared_from_this(),
                              is_write_mode);
  }

#ifdef TRISYCL_OPENCL
  /// Returns the cl_buffer for a given context.
  boost::compute::buffer get_cl_buffer(cl::sycl::context context) {
    return buffer_cache[context];
  }


  /** Create the OpenCL buffer and copy in data from the host if it doesn't
   already exists for a given context or if the buffer is in write mode */
  void copy_in_cl_buffer(cl::sycl::context context,
                         cl_mem_flags flags, bool is_write,
                         std::size_t size, void* data_ptr) {
    if (buffer_cache.find(context) == buffer_cache.end() || is_write) {
      buffer_cache[context] = boost::compute::buffer
        { context.get_boost_compute(), size, flags, data_ptr };
    }
  }

  /// Copy back the CL buffer to the SYCL host if required
  void copy_back_cl_buffer(boost::compute::command_queue q,
                           cl::sycl::context context, std::size_t size,
                           void* data_ptr) {
    q.enqueue_read_buffer(buffer_cache[context],
                          0 /*< Offset */,
                          size,
                          data_ptr);
  }
#endif

};

/// @} End the data Doxygen group

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
