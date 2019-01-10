#ifndef TRISYCL_SYCL_TASK_HPP
#define TRISYCL_SYCL_TASK_HPP

/** \file The concept of task behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
#include <boost/thread/lock_algorithms.hpp>

#include "CL/sycl/accessor/detail/accessor_base.hpp"
#include "CL/sycl/buffer/detail/buffer_base.hpp"
#include "CL/sycl/detail/debug.hpp"
#include "CL/sycl/kernel.hpp"
#include "CL/sycl/queue/detail/queue.hpp"

namespace cl::sycl::detail {

/** The abstraction to represent SYCL tasks executing inside command_group

    "enable_shared_from_this" allows to access the shared_ptr behind the
    scene.
*/
struct task : public std::enable_shared_from_this<task>,
              public detail::debug<task> {

  /// List of the buffers used by this task and their write status
  std::map<std::shared_ptr<detail::buffer_base>, bool> buffers_in_use;

  /// The tasks producing the buffers used by this task
  std::vector<std::shared_ptr<detail::task>> producer_tasks;

  /// Keep track of any prologue to be executed before the kernel
  std::vector<std::function<void(void)>> prologues;

  /// Keep track of any epilogue to be executed after the kernel
  std::vector<std::function<void(void)>> epilogues;

  /// Store if the execution ended, to be notified by task_ready
  bool execution_ended = false;

  /// To signal when this task is ready
  std::condition_variable ready;

  /// To protect the access to the condition variable
  std::mutex ready_mutex;

  /** Keep track of the queue used to submission to notify kernel completion
      or to run OpenCL kernels on */
  std::shared_ptr<detail::queue> owner_queue;

  /// The functor defining the task content to be executed
  std::function<void(void)> content;

  /// The OpenCL-compatible kernel run by this task, if any
  std::shared_ptr<detail::kernel> kernel;

  /** The accessors indexed by their creation order

      This is used to relate a kernel parameter of a kernel generated
      by the device compiler to its accessor. */
  std::vector<std::weak_ptr<detail::accessor_base>> accessors;


  /// Create a task from a submitting queue
  task(const std::shared_ptr<detail::queue> &q)
    : owner_queue { q } {}


  /// Add a content to be executed by the task
  void schedule(std::function<void(void)> f) {
    // \todo test and throw if there is already something to execute
    content = f;
  }


  /// Finalize the task by inserting it in the task graph
  void finalize() {
    register_buffers_in_task_graph();
    /* To keep a copy of the task shared_ptr after the end of the
       command group, capture it by copy in the following lambda. This
       should be easier in C++17 with move semantics on capture
    */
    auto task = shared_from_this();
    auto execution = [=] {
      // Wait for the required tasks to be ready
      task->wait_for_producers();
      task->prelude();
      TRISYCL_DUMP_T("Execute the kernel");
      // Execute the kernel
      content();
      task->postlude();
      // Release the buffers that have been written by this task
      task->release_buffers();
      // Notify the waiting tasks that we are done
      task->notify_consumers();
      // Notify the queue we are done
      owner_queue->kernel_end();
      /* Remove the functor that might otherwise keep references to
         buffers indirectly and have a circular dependency preventing
         the final release */
      task->content = nullptr;
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
#ifndef TRISYCL_NO_ASYNC
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


  /// Release the buffers that have been used by this task
  void release_buffers() {
    TRISYCL_DUMP_T("Task " << this << " releases the written buffers");
    for (auto &b: buffers_in_use)
      b.first->release();
    buffers_in_use.clear();
  }


  /// Notify the waiting tasks that we are done
  void notify_consumers() {
    TRISYCL_DUMP_T("Notify all the task waiting for this task " << this);
   {
     std::unique_lock<std::mutex> ul { ready_mutex };
     execution_ended = true;
   }
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
  void add_buffer(std::shared_ptr<detail::buffer_base> &buf,
                  bool is_write_mode) {
    TRISYCL_DUMP_T("Add buffer " << buf << " in task " << this);
    if (is_write_mode)
      // Always record the write status
      buffers_in_use.emplace(buf, is_write_mode);
    else
      // For the read case, update if nothing yet, to let the write case win
      buffers_in_use.try_emplace(buf, is_write_mode);
  }


  /// Register all the buffers in an atomic way
  void register_buffers_in_task_graph() {
    /* Construct a list of unique locks for dependency graph
       associated to buffers */
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto &b : buffers_in_use)
      // Create a lock for the mutex, but do not lock it yet
      locks.emplace_back(b.first->get_dependency_graph_construction_mutex(),
                         std::defer_lock);
    /* Lock all the required buffers for the dependency graph
       construction to avoid deadlock if there is another thread doing
       the same thing.

       Look at tests/queue/deadlock.cpp use-case
    */
    for (;;) {
      if (boost::try_lock(locks.begin(), locks.end()) == locks.end()) {
        for (auto &b : buffers_in_use)
          register_buffer(b.first, b.second);
        break;
      }
    }
    locks.clear();
  }


  /** Register a buffer to this task

      This is how the dependency graph is incrementally built.
  */
  void register_buffer(const std::shared_ptr<detail::buffer_base> &buf,
                       bool is_write_mode) {
    TRISYCL_DUMP_T("register_buffer " << buf << " in task " << this);
    // To be sure the buffer does not disappear before the kernel can run
    buf->use();

    std::shared_ptr<detail::task> latest_producer;
    if (is_write_mode) {
      /* Set this task as the latest producer of the buffer so that
         another kernel may wait on this task */
      latest_producer = buf->set_latest_producer(shared_from_this());
    }
    else
      latest_producer = buf->get_latest_producer();

    /* If the buffer is to be produced by a task, add the task in the
       producer list to wait on it before running the task core */
    if (latest_producer) {
      assert(latest_producer != shared_from_this());
      producer_tasks.push_back(latest_producer);
    }
  }


  /// Execute the prologues
  void prelude() {
    TRISYCL_DUMP_T("task::prelude");

    for (const auto &p : prologues)
      p();
    /* Free the functors that may own an accessor owning a buffer
       preventing the command group to complete */
    prologues.clear();
  }


  /// Execute the epilogues
  void postlude() {
    for (const auto &p : epilogues)
      p();
    /* Free the functors that may own an accessor owning a buffer
       preventing the command group to complete */
    epilogues.clear();
  }


  /// Add a function to the prelude to run before kernel execution
  void add_prelude(const std::function<void(void)> &f) {
    TRISYCL_DUMP_T("task::add_prelude");

    prologues.push_back(f);
  }


  /// Add a function to the postlude to run after kernel execution
  void add_postlude(const std::function<void(void)> &f) {
    epilogues.push_back(f);
  }


  /// Get the queue behind the task to run a kernel on
  auto get_queue() {
    return owner_queue;
  }


  /// Set the OpenCL-compatible kernel running this task if any
  void set_kernel(const std::shared_ptr<cl::sycl::detail::kernel> &k) {
    kernel = k;
  }


  /** Get the kernel running if any

      \todo Specify this error in the spec
  */
  cl::sycl::detail::kernel &get_kernel() {
    if (!kernel)
      throw non_cl_error("Cannot use an OpenCL kernel in this context");
    return *kernel;
  }


  /// Set a kernel argument by address
  void set_arg(int arg_index, std::size_t arg_size, const void *scalar_value) {
#ifdef TRISYCL_OPENCL
    // Forward to the OpenCL kernel
    get_kernel().get_boost_compute().set_arg(arg_index,
                                             arg_size,
                                             scalar_value);
#else
    throw non_cl_error("Not compiled with OpenCL support");
#endif
  }


  /// Set a kernel argument by value
  template <typename T>
  void set_arg(int arg_index, const T &scalar_value) {
#ifdef TRISYCL_OPENCL
    // Forward to the OpenCL kernel
    get_kernel().get_boost_compute().set_arg(arg_index, scalar_value);
#else
    throw non_cl_error("Not compiled with OpenCL support");
#endif
  }


#ifdef TRISYCL_OPENCL
  /// Get the Boost.Compute buffer for an accessor of the task
  auto get_compute_buffer(std::size_t order) {
    return accessors[order].lock()->get_cl_buffer();
  }
#endif


/** Register an accessor and return its registration order

    I would prefer to name this method \c register but this is a C++
    keyword...
*/
std::size_t register_accessor(std::weak_ptr<detail::accessor_base> a) {
  accessors.push_back(a);
  return accessors.size() - 1;
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

#endif // TRISYCL_SYCL_TASK_HPP
