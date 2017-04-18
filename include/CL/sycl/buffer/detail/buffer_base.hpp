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
#include <unordered_set>

// \todo Use C++17 optional when it is mainstream
#include <boost/optional.hpp>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
#include "CL/sycl/context.hpp"
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

  /// To track contexts in which the data is up to date
  std::unordered_set<cl::sycl::context> fresh_ctx;

#ifdef TRISYCL_OPENCL
  /** Buffer side cache that keeps the boost::compute::buffer (and the
      underlying cl_buffer) so that if the buffer already exists inside
      the same context it is not recreated.
   */
  std::unordered_map<cl::sycl::context, boost::compute::buffer> buffer_cache;
#endif

  /** Create a buffer base and marks the host context as the context that
      holds the most recent version of the data
   */
  buffer_base() : number_of_users { 0 },
                  fresh_ctx { cl::sycl::context {} } {}


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
  /// Check if the data of this buffer is up-to-date in a certain context
  bool data_is_up_to_date(cl::sycl::context ctx) {
    return fresh_ctx.find(ctx) != fresh_ctx.end();
  }


  /// Check if the buffer is already cached for a certain context
  bool is_cached(cl::sycl::context ctx) {
    return buffer_cache.find(ctx) != buffer_cache.end();
  }


  /** Create a boost::compute::buffer for this cl::sycl::buffer in the
      cache and associate it with a given context
  */
  void create_in_cache(cl::sycl::context ctx, size_t size,
                       cl_mem_flags flags, void* data) {
    buffer_cache[ctx] = boost::compute::buffer
      { ctx.get_boost_compute(),
        size,
        flags,
        data
      };
  }


  /** Transfer the most up-to-date version of the data to the host
      if the host is not already up-to-date
  */
  void sync_with_host(std::size_t size, void* data) {
    cl::sycl::context host_context {};
    if(!data_is_up_to_date(host_context) && fresh_ctx.size() > 0) {
      // We know that the context(s) in fresh_ctx hold the most recent version
      // of the buffer
      auto fresh_context = *(fresh_ctx.begin());
      auto fresh_q = fresh_context.get_boost_queue();
      fresh_q.enqueue_read_buffer(buffer_cache[fresh_context], 0, size, data);
      fresh_ctx.insert(host_context);
    }
  }


  /** When a new accessor is created this function is called, it will
      update the state of the buffer according to the context for which
      the accessor is created and the access mode
  */
  void update_fresh(cl::sycl::context target_ctx, access::mode mode,
                    std::size_t size, void* data) {
    // If read mode and the data is up-to-date there is nothing to do
    if(mode == access::mode::read && data_is_up_to_date(target_ctx)) return;

    if(mode == access::mode::read) {
      // The data is not up-to-date, we need a transfer
      // We also want to be sure that the host holds the most recent data
      sync_with_host(size, data);

      if(!target_ctx.is_host()) {
        // If the target context is a device context
        if(!is_cached(target_ctx)) {
          /* If not cached, we create the buffer in READ_ONLY mode and copy
             the data at the same time
          */
          create_in_cache(target_ctx, size,
                          (CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR), data);
          fresh_ctx.insert(target_ctx);
          return;
        }

        /* Else we transfer the data to the existing buffer associated
           with the target context buffer
        */
        boost::compute::command_queue& q = target_ctx.get_boost_queue();
        q.enqueue_write_buffer(buffer_cache[target_ctx], 0, size, data);
        fresh_ctx.insert(target_ctx);
      }
      return;
    }


    if((   mode == access::mode::read_write
        || mode == access::mode::write
        || mode == access::mode::atomic) && !data_is_up_to_date(target_ctx)) {
      // If the data is not up-to-date in the target context
      // We want to host to be up-to-date
      sync_with_host(size, data);

      if(!target_ctx.is_host()) {
        // If the target context is a device context
        if(!is_cached(target_ctx)) {
          /* If not cached, we create the buffer in the corresponding mode
             and copy the data at the same time.
          */
          cl_mem_flags flags = (mode == access::mode::write) ?
            CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE;
          // We indicate we want the data to be copied upon creation
          flags |= CL_MEM_COPY_HOST_PTR;
          create_in_cache(target_ctx, size, flags, data);
        }

        else {
          // We update the buffer associated with the target context
          boost::compute::command_queue q = target_ctx.get_boost_queue();
          q.enqueue_write_buffer(buffer_cache[target_ctx], 0, size, data);
        }
      }
    }

    if((   mode == access::mode::discard_write
        || mode == access::mode::discard_read_write)
        && !data_is_up_to_date(target_ctx)) {
      /* We only need to create the buffer if it doesn't exist
         but without copying any data because of the discard mode
      */
      if(!target_ctx.is_host() && !is_cached(target_ctx)) {
        // If the context doesn't exist we create it
        cl_mem_flags flags = (mode == access::mode::discard_write) ?
          CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE;
        // We don't want to transfer any data
        create_in_cache(target_ctx, size, flags, 0);
      }
    }

    /* If the accessor can write to the buffer, we indicate that all
       contexts except the target context are not up-to-date anymore
    */
    fresh_ctx.clear();
    fresh_ctx.insert(target_ctx);
  }


  /// Returns the cl_buffer for a given context.
  boost::compute::buffer get_cl_buffer(cl::sycl::context context) {
    return buffer_cache[context];
  }


  /** Create the OpenCL buffer and copy in data from the host if it doesn't
      already exists for a given context
  */
  void copy_in_cl_buffer(boost::compute::command_queue q,
                         cl::sycl::context context,
                         cl_mem_flags flags, std::size_t size, void* data) {
    // The data in the buffer is up to date, nothing to do
    if(data_is_up_to_date(context)) return;

    // We want to be sure that the host hold the most recent version of the data
    sync_with_host(size, data);

    if(is_cached(context)) {
      // If the data in the context is not up to date, we update it
      q.enqueue_write_buffer(buffer_cache[context], 0, size, data);
      fresh_ctx.insert(context);
      return;
    }

    // The buffer was never used in this context, we create the cached buffer
    create_in_cache(context, size, flags, data);
    fresh_ctx.insert(context);
  }


  /// Copy back the CL buffer to the SYCL host if required
  void copy_back_cl_buffer(boost::compute::command_queue q,
                           cl::sycl::context context, std::size_t size,
                           void* data) {
    // host context
    cl::sycl::context host_context {};

    // if the data is clean on the host context, nothing to do
    if(data_is_up_to_date(host_context)) return;

    // Otherwise we just update the data on the host context
    sync_with_host(size, data);
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
