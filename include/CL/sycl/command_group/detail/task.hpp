#ifndef TRISYCL_SYCL_TASK_HPP
#define TRISYCL_SYCL_TASK_HPP

/** \file The concept of task behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>
#include <thread>

#include "CL/sycl/access.hpp"
#include "CL/sycl/buffer/detail/buffer_base.hpp"
#include "CL/sycl/buffer/detail/buffer_customer.hpp"
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
  /// The buffers that are used by this task
  std::vector<std::shared_ptr<buffer_customer>> buffers;

  /// Add a new task to the task graph and schedule for execution
  void schedule(std::function<void(void)> f) {
    /** To keep a copy of the task shared_ptr after the end of the command
        group, capture it by copy in the following lambda. This should be
        easier in C++17 with move semantics on capture
    */
    auto task = shared_from_this();
    auto execution = [=] {
      // Wait for the required buffers to be ready
      task->acquire_buffers();
      TRISYCL_DUMP_T("Execute the kernel");
      // Execute the kernel
      f();
      // Release the required buffers for other uses
      task->release_buffers();
      TRISYCL_DUMP_T("Exit");
    };
#if TRISYCL_ASYNC
    /* If in asynchronous execution mode, execute the functor in a new
       thread */
    std::thread thread(execution);
    TRISYCL_DUMP_T("Started");
    // Detach the thread since it will synchronize by its own means
    thread.detach();
#else
    // Just a synchronous execution otherwise
    execution();
#endif
  }


  void acquire_buffers() {
    TRISYCL_DUMP_T("acquire_buffers()");
    for (auto &b : buffers)
      b->wait();
  }


  void release_buffers() {
    TRISYCL_DUMP_T("release_buffers()");
    for (auto &b : buffers)
      b->release();
  }


  /** Register an accessor to this task

      This is how the dependency graph is incrementally built.
  */
  template <typename T,
            std::size_t dimensions,
            access::mode mode,
            access::target target = access::global_buffer>
  void add(AccessorImpl<T, dimensions, mode, target> &a) {
    auto bc = buffer_base::get_buffer_customer(a);
    // Add the task as a new client for the buffer customer of the accessor
    bc->add(shared_from_this(), a.isWriteAccess());
    buffers.push_back(bc);
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
