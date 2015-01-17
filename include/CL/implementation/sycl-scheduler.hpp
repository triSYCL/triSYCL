#if TRISYCL_ASYNC
#include <thread>
#endif

namespace cl {
namespace sycl {
namespace trisycl {

template <typename T,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct AccessorImpl;


/** The abstraction to represent SYCL tasks executing inside command_group

    "enable_shared_from_this" allows to access the shared_ptr behind the
    scene.
 */
struct Task : std::enable_shared_from_this<Task>,
              public Debug<Task> {
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


  /// Register an accessor to this task
  template <typename T,
            std::size_t dimensions,
            access::mode mode,
            access::target target = access::global_buffer>
  void add(AccessorImpl<T, dimensions, mode, target> &A) {
    // Add the task as a new client for the buffer of the accessor
    A.getBuffer().addClient(A, shared_from_this());
  }

};


/** Keep track of the tasks waiting for the availability of a buffer
    generation, either to read it or to write it
*/
class BufferCustomer : public Debug<BufferCustomer> {

  bool WriteAccess;
  std::vector<std::shared_ptr<Task>> tasks;

public:

  /// Add a new task as a customer
  void add(std::shared_ptr<Task> task, bool writeAccess) {
    WriteAccess = writeAccess;
    tasks.push_back(task);
  }

};


/** Store the current Task to attach the task and accessors to it.

    Use a TLS variable since there may be several threads in the program.

    \todo Change the specification to avoid this hack.
*/
thread_local std::shared_ptr<Task> CurrentTask;


struct CommandGroupImpl : public Debug<CommandGroupImpl> {


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
