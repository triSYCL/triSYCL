#if TRISYCL_ASYNC
#include <thread>
#endif

namespace cl {
namespace sycl {
namespace trisycl {

struct Task {
  /// Add a new task to the task graph
  static void add(std::function<void(void)> f) {
#if TRISYCL_ASYNC
    // Execute the functor in a new thread
    std::thread thread(f);
    // std::cout << thread.get_id();
    // For now just wait for the synchronous execution
    thread.join();
#else
    // Just a synchronous execution
    f();
#endif
  }

};


/** Store the current CommandGroupImpl to attach the task and accessors to it.

    Use a TLS variable since there may be several threads in the program.

    \todo Change the specification to avoid this hack.
*/
thread_local CommandGroupImpl *current_command_group = nullptr;


struct CommandGroupImpl {
  template <typename Functor>
  CommandGroupImpl(queue Q, Functor F) {
    // Nesting of command_group is forbidden
    assert(current_command_group == nullptr);
    current_command_group = this;
    F();
  }


  ~CommandGroupImpl() {
    // Reset the current_command_group at the end of the command_group
    current_command_group = nullptr;
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
