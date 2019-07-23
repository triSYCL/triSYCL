#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_BASE_HPP

/** \file The buffer_base behind the buffers, independent of the data
    type

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
// \todo Use C++17 optional when it is mainstream
#include <boost/optional.hpp>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <utility>

#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/context.hpp"

namespace trisycl {

class handler;

namespace detail {

struct task;
struct buffer_base;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

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

  /// To track contexts in which the data is up-to-date
  std::unordered_set<trisycl::context> fresh_ctx;

#ifdef TRISYCL_OPENCL
  /** Buffer-side cache that keeps the \c boost::compute::buffer (and the
      underlying \c cl_buffer ) so that if the buffer already exists inside
      the same context it is not recreated.
   */
  std::unordered_map<trisycl::context, boost::compute::buffer> buffer_cache;
#endif

  /** Create a buffer base and marks the host context as the context that
      holds the most recent version of the data
      \todo Use lazy allocation for the context tracking set
   */
  buffer_base() : number_of_users { 0 },
                  fresh_ctx { trisycl::context {} } {}


  /// The destructor waits for not being used anymore
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
    std::unique_lock<std::mutex> lock { ready_mutex };
    if (--number_of_users == 0) {
      // Micro-optimization: unlock before the notification
      // https://en.cppreference.com/w/cpp/thread/condition_variable/notify_all
      lock.unlock();
      // Notify the host consumers or the buffer destructor that it is ready
      ready.notify_all();
    }
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
  /// Check if the data of this buffer is up-to-date in a certain context
  bool is_data_up_to_date(const trisycl::context& ctx) {
    return fresh_ctx.count(ctx);
  }


  /// Check if the buffer is already cached for a certain context
  bool is_cached(const trisycl::context& ctx) {
    return buffer_cache.count(ctx);
  }


  /** Create a \c boost::compute::buffer for this \c trisycl::buffer in the
      cache and associate it with a given context
  */
  void create_in_cache(const trisycl::context& ctx, size_t size,
                       cl_mem_flags flags, void* data) {
    buffer_cache[ctx] = boost::compute::buffer
      { ctx.get_boost_compute(),
        size,
        flags,
        data
      };
  }


  /** Transfer the most up-to-date version of the data to the host
      if the host version is not already up-to-date
  */
  void sync_with_host(std::size_t size, void* data) {
    trisycl::context host_context;
    if (!is_data_up_to_date(host_context) && !fresh_ctx.empty()) {
      /* We know that the context(s) in \c fresh_ctx hold the most recent
         version of the buffer
      */
      auto fresh_context = *(fresh_ctx.begin());
      auto fresh_q = fresh_context.get_boost_queue();
      fresh_q.enqueue_read_buffer(buffer_cache[fresh_context], 0, size, data);
      fresh_ctx.insert(host_context);
    }
  }


  /** When a transfer is requested this function is called, it will
      update the state of the buffer according to the context in which
      the accessor is created and the access mode
  */
  void update_buffer_state(const trisycl::context& target_ctx,
                           access::mode mode, std::size_t size, void* data) {
    /* The \c cl_buffer we put in the cache might get accessed again in the
       future, this means that we have to always to create it in read/write
       mode to be able to write to it if it is accessed through a
       write accessor in the future
     */
    auto constexpr flag = CL_MEM_READ_WRITE;

    /* The buffer is accessed in read mode, we want to transfer the data only if
       necessary. We start a transfer if the data on the target context is not
       up to date and then update the fresh context set.
     */
    if (mode == access::mode::read) {

      if (is_data_up_to_date(target_ctx))
        // If read mode and the data is up-to-date there is nothing to do
        return;

      // The data is not up-to-date, we need a transfer
      // We also want to be sure that the host holds the most recent data
      sync_with_host(size, data);

      if (!target_ctx.is_host()) {
        // If the target context is a device context
        if (!is_cached(target_ctx)) {
          /* If not cached, we create the buffer and copy the data
             at the same time
          */
          create_in_cache(target_ctx, size,
                          (flag | CL_MEM_COPY_HOST_PTR), data);
          fresh_ctx.insert(target_ctx);
          return;
        }

        /* Else we transfer the data to the existing buffer associated
           with the target context buffer
        */
        auto q = target_ctx.get_boost_queue();
        q.enqueue_write_buffer(buffer_cache[target_ctx], 0, size, data);
        fresh_ctx.insert(target_ctx);
      }
      return;
    }

    /* The buffer might be written to, this means that we have to consider
       every version of the data obsolete except in the target context

       We go through the same process as in read mode but in addition
       we empty the fresh context set and just add the target context

       If the data is up to date on the target we just have to update
       the context set and nothing else
    */
    if (!is_data_up_to_date(target_ctx)) {

      if (   mode == access::mode::read_write
          || mode == access::mode::write
          || mode == access::mode::atomic) {
        // If the data is not up-to-date in the target context
        // We want to host to be up-to-date
        sync_with_host(size, data);

        if (!target_ctx.is_host()) {
          // If the target context is a device context
          if (!is_cached(target_ctx)) {
            create_in_cache(target_ctx, size,
                            (flag | CL_MEM_COPY_HOST_PTR), data);
          }
          else {
            // We update the buffer associated with the target context
            auto q = target_ctx.get_boost_queue();
            q.enqueue_write_buffer(buffer_cache[target_ctx], 0, size, data);
          }
        }
      }

      /* When in discard mode we don't need to transfer any data, we just create
         the \c cl_buffer if it doesn't exist in the cache
      */
      if (   mode == access::mode::discard_write
          || mode == access::mode::discard_read_write) {
        /* We only need to create the buffer if it doesn't exist
           but without copying any data because of the discard mode
        */
        if (!target_ctx.is_host() && !is_cached(target_ctx)) {
          // If the context doesn't exist we create it.
          /* We don't want to transfer any data so we don't
             add \c CL_MEM_COPY_HOST_PTR
          */
          create_in_cache(target_ctx, size, flag, 0);
        }
      }
    }
    /* Here we are sure that we are in some kind of write mode,
       we indicate that all contexts except the target context
       are not up-to-date anymore
    */
    fresh_ctx.clear();
    fresh_ctx.insert(target_ctx);
  }


  /// Returns the cl_buffer for a given context.
  boost::compute::buffer get_cl_buffer(const trisycl::context& context) {
    return buffer_cache[context];
  }

#endif

};

/// @} End the data Doxygen group

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
