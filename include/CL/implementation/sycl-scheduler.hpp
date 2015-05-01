namespace cl {
namespace sycl {
namespace trisycl {

/** The abstraction to represent SYCL tasks executing inside command_group

    "enable_shared_from_this" allows to access the shared_ptr behind the
    scene.
 */
struct Task : std::enable_shared_from_this<Task>,
              public detail::debug<Task> {
  /// The buffers that are used by this task
  std::vector<std::shared_ptr<BufferCustomer>> Buffers;

  /// Add a new task to the task graph and schedule for execution
  void schedule(std::function<void(void)> F) {
    /** To keep a copy of the Task shared_ptr after the end of the command
        group, capture it by copy in the following lambda. This should be
        easier in C++17 with move semantics on capture
    */
    auto task = shared_from_this();
    auto execution = [=] {
      // Wait for the required buffers to be ready
      task->acquireBuffers();
      TRISYCL_DUMP_T("Execute the kernel");
      // Execute the kernel
      F();
      // Release the required buffers for other uses
      task->releaseBuffers();
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


  void acquireBuffers() {
    TRISYCL_DUMP_T("acquireBuffers()");
    for (auto &b : Buffers)
      b->wait();
  }


  void releaseBuffers() {
    TRISYCL_DUMP_T("releaseBuffers()");
    for (auto &b : Buffers)
      b->release();
  }


  /** Register an accessor to this task

      This is how the dependency graph is incrementally built.
  */
  template <typename T,
            std::size_t dimensions,
            access::mode mode,
            access::target target = access::global_buffer>
  void add(AccessorImpl<T, dimensions, mode, target> &A) {
    auto BC = BufferBase::getBufferCustomer(A);
    // Add the task as a new client for the buffer customer of the accessor
    BC->add(shared_from_this(), A.isWriteAccess());
    Buffers.push_back(BC);
  }

};


/** Store the current Task to attach the task and accessors to it.

    Use a TLS variable since there may be several threads in the program.

    \todo Change the specification to avoid this hack.
*/
thread_local std::shared_ptr<Task> CurrentTask;


struct CommandGroupImpl : public detail::debug<CommandGroupImpl> {


  template <typename Queue, typename Functor>
  CommandGroupImpl(Queue Q, Functor F) {
    // Nesting of command_group is forbidden, so there should be no
    // current task yet
    assert(!CurrentTask);
    // Create a new task for this command_group
    CurrentTask = std::make_shared<Task>();
    F();
  }


  ~CommandGroupImpl() {
    // There should be a current task
    assert(CurrentTask);
    // Reset the current_command_group at the end of the command_group
    CurrentTask.reset();
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
