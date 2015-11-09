#ifndef TRISYCL_SYCL_TASK_HPP
#define TRISYCL_SYCL_TASK_HPP

/** \file The concept of task behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <condition_variable>
#include <memory>
#include <thread>

#include "CL/sycl/access.hpp"
#include "CL/sycl/buffer/detail/buffer.hpp"
#include "CL/sycl/detail/debug.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** The abstraction to represent SYCL tasks executing inside command_group

    "enable_shared_from_this" allows to access the shared_ptr behind the
    scene.
*/
struct task : std::enable_shared_from_this<task>,
              public detail::debug<task> {
  /// The tasks producing the buffers used by this task
  std::vector<std::shared_ptr<detail::task>> producer_tasks;

  /// List of buffer written by this task
  std::vector<detail::buffer_base *> written_buffers;

  /// Store if the execution ended, to be notified by task_ready
  bool execution_ended = false;

  /// To signal when this task is ready
  std::condition_variable ready;

  /// To protect the access to the condition variable
  std::mutex ready_mutex;


  /// Add a new task to the task graph and schedule for execution
  void schedule(std::function<void(void)> f) {
    /* To keep a copy of the task shared_ptr after the end of the
       command group, capture it by copy in the following lambda. This
       should be easier in C++17 with move semantics on capture
    */
    auto task = shared_from_this();
    auto execution = [=] {
      // Wait for the required tasks to be ready
      task->wait_for_producers();
      TRISYCL_DUMP_T("Execute the kernel");
      // Execute the kernel
      f();
      // Release the buffers that have been written by this task
      task->release_written_buffers();
      // Notify the waiting tasks that we are done
      task->notify_consumers();
      TRISYCL_DUMP_T("Exit");
    };
    /* \todo it may be implementable with packaged_task that would
       deal with exceptions in kernels
    */
#if TRISYCL_ASYNC
    /* If in asynchronous execution mode, execute the functor in a new
       thread */
    std::thread thread(execution);
    TRISYCL_DUMP_T("Task thread started");
    /** Detach the thread since it will synchronize by its own means

        \todo This is an issue if there is an exception in the kernel
    */
    thread.detach();
#else
    // Just a synchronous execution otherwise
    execution();
#endif
  }


  /// Wait for the required producer tasks to be ready
  void wait_for_producers() {
  TRISYCL_DUMP_T("Wait for the producer tasks");
    for (auto &t : producer_tasks)
      t->wait();
    // We can let the producers rest in peace
    /* \todo Think more about who is retaining the input buffer and
       their life time */
    producer_tasks.clear();
  }


  /// Release the buffers that have been written by this task
  void release_written_buffers() {
  TRISYCL_DUMP_T("Release the written buffers");
    for (auto b : written_buffers)
      b->release(this);
    written_buffers.clear();
  }


  /// Notify the waiting tasks that we are done
  void notify_consumers() {
    TRISYCL_DUMP_T("Notify all the task waiting for us");
    execution_ended = true;
    /* \todo Verify that the memory model with the notify does not
       require some fence or atomic */
    ready.notify_all();
  }


  /** Wait for this task to be ready

      This is to be called from another thread
  */
  void wait() {
      std::unique_lock<std::mutex> ul { ready_mutex };
      ready.wait(ul, [&] { return execution_ended; });
  }


  /** Register a buffer to this task

      This is how the dependency graph is incrementally built.
  */
  void add_buffer(detail::buffer_base *buf, bool is_write_mode) {
    detail::task *latest_producer {};

    if (is_write_mode) {
      // Keep track of the written buffer to notify some host consumers later
      written_buffers.push_back(buf);
      /* Set this task as the latest producer of the buffer so that
         another kernel mat wait on this task */
      latest_producer = buf->set_latest_producer(this);
    }
    else
      latest_producer = buf->get_latest_producer();

    /* If the buffer is to be produced by a task, add the task in the
       producer list to wait on it before running the task core */
    if (latest_producer)
      producer_tasks.push_back(latest_producer->shared_from_this());
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

#endif // TRISYCL_SYCL_TASK_HPP
