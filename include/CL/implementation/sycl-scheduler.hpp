#if TRISYCL_ASYNC
#include <thread>
#endif

namespace cl {
namespace sycl {
namespace trisycl {

struct Task {
  /// Add a new task to the task graph
  static void add(std::function<void(void)> F) {
#if TRISYCL_ASYNC
    // Execute the functor in a new thread
    std::thread thread(F);
    // std::cout << thread.get_id();
    // For now just wait for the synchronous execution
    thread.join();
#else
    // Just a synchronous execution
    F();
#endif
  }

};


/** Store the current Task to attach the task and accessors to it.

    Use a TLS variable since there may be several threads in the program.

    \todo Change the specification to avoid this hack.
*/
thread_local std::shared_ptr<Task> CurrentTask;


struct CommandGroupImpl : public Debug<CommandGroupImpl> {


  template <typename Functor>
  CommandGroupImpl(queue Q, Functor F) {
    // Nesting of command_group is forbidden, so there should be no
    // current task yet
    assert(!CurrentTask);
    CurrentTask = std::make_shared<Task>();
    F();
  }


  ~CommandGroupImpl() {
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
