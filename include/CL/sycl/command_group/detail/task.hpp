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

#include "CL/sycl/buffer/detail/buffer_base.hpp"
#include "CL/sycl/detail/debug.hpp"
#include "CL/sycl/queue/detail/queue.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** The abstraction to represent SYCL tasks executing inside command_group

    "enable_shared_from_this" allows to access the shared_ptr behind the
    scene.
*/
struct task : public std::enable_shared_from_this<task>,
              public detail::debug<task> {

  /** List of the buffers used by this task

      \todo Use a set to check that some buffers are not used many
      times at least on writing
  */
  std::vector<detail::buffer_base *> buffers_in_use;

  /// The tasks producing the buffers used by this task
  std::vector<std::shared_ptr<detail::task>> producer_tasks;

  /// Store if the execution ended, to be notified by task_ready
  bool execution_ended = false;

  /// To signal when this task is ready
  std::condition_variable ready;

  /// To protect the access to the condition variable
  std::mutex ready_mutex;

  /// Keep track of the queue used to submission to notify kernel completion
  detail::queue *owner_queue;


  /// Create a task from a submitting queue
  task(detail::queue &q) : owner_queue { &q } {}


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
      task->release_buffers();
      // Notify the waiting tasks that we are done
      task->notify_consumers();
      // Notify the queue we are done
      owner_queue->kernel_end();
      TRISYCL_DUMP_T("Task thread exit");
    };
    /* Notify the queue that there is a kernel submitted to the
       queue. Do not do it in the task contructor so that we can deal
       with command group without kernel and if we put it inside the
       thread, the queue may have finished before the thread is
       scheduled */
    owner_queue->kernel_start();
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
    TRISYCL_DUMP_T("Task " << this << " waits for the producer tasks");
    for (auto &t : producer_tasks)
      t->wait();
    // We can let the producers rest in peace
    producer_tasks.clear();
  }


  /// Release the buffers that have  been used by this task
  void release_buffers() {
    TRISYCL_DUMP_T("Task " << this << " releases the written buffers");
    for (auto b: buffers_in_use)
      b->release();
    buffers_in_use.clear();
  }


  /// Notify the waiting tasks that we are done
  void notify_consumers() {
    TRISYCL_DUMP_T("Notify all the task waiting for this task " << this);
    execution_ended = true;
    /* \todo Verify that the memory model with the notify does not
       require some fence or atomic */
    ready.notify_all();
  }


  /** Wait for this task to be ready

      This is to be called from another thread
  */
  void wait() {
    TRISYCL_DUMP_T("The task wait for task " << this << " to end");
    std::unique_lock<std::mutex> ul { ready_mutex };
    ready.wait(ul, [&] { return execution_ended; });
  }


  /** Register a buffer to this task

      This is how the dependency graph is incrementally built.
  */
  void add_buffer(detail::buffer_base *buf, bool is_write_mode) {
    detail::task *latest_producer {};

    /* Keep track of the use of the buffer to notify its release at
       the end of the execution */
    buffers_in_use.push_back(buf);
    // To be sure the buffer does not disappear before the kernel can run
    buf->use();
    TRISYCL_DUMP_T("Add buffer " << buf << " in task " << this);

    if (is_write_mode) {
      /* Set this task as the latest producer of the buffer so that
         another kernel may wait on this task */
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
