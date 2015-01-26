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
class Task;


/** Keep track of the tasks waiting for the availability of a buffer
    generation, either to read it or to write it

    When we write into a buffer, we generate a new version of it (think
    "SSA")
*/
class BufferCustomer : public Debug<BufferCustomer> {
  BufferBase &Buffer;
  // At some point use lock free list for this inside BufferBase
  std::shared_ptr<BufferCustomer> nextGeneration;
  // Needed?
  bool WriteAccess;
  bool ReadyToUse;
  std::mutex ReadyMutex;
  std::condition_variable ReadyCV;
  std::atomic<unsigned int> UserNumber;

public:

  BufferCustomer(BufferBase &Buffer, bool isWriteAccess)
    : Buffer { Buffer },  WriteAccess { isWriteAccess },
      ReadyToUse { false }, UserNumber { 0 } {
  }


  /// Get the buffer customer associated to the latest version of the buffer
  template <typename T,
            std::size_t dimensions,
            access::mode mode,
            access::target target = access::global_buffer>
  static std::shared_ptr<BufferCustomer>
  getBufferCustomer(AccessorImpl<T, dimensions, mode, target> &A) {
    BufferBase &B = A.getBuffer();
    {
      /// Use atomic list?
      // Protect the update of LastBufferCustomer in the Buffer
      auto Lock = B.lock();
      std::shared_ptr<BufferCustomer> BC = B.getLastBufferCustomer();
      auto OldBC = BC;
      /* When we write into a buffer, we generate a new version of it (think
         "SSA"). Of course we do it also when there is not yet any
         BufferCustomer */
      if (!BC || A.isWriteAccess()) {
        BC = std::make_shared<BufferCustomer>(B, A.isWriteAccess());
        B.setLastBufferCustomer(BC);
      }

      if (OldBC)
        // \todo Use atomic list instead
        OldBC->nextGeneration = BC;
      else
        // If we just created the BufferCustomer, it is ready to use
        BC->notifyReady();

      return BC;
    }
  }


  /// Add a new task as a customer of the buffer generation
  void add(std::shared_ptr<Task> task, bool writeAccess) {
    WriteAccess = writeAccess;
    UserNumber++;
  }


  /// Wait for the buffer generation to be ready to use
  void wait() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      ReadyCV.wait(UL, [&] { return ReadyToUse; });
    }
  }


  /// Release the buffer generation usage
  void release() {
    UserNumber--;
    /* If there is no task using this generation of the buffer, make the
       next generation ready if any*/
    if (UserNumber == 0 && nextGeneration)
      nextGeneration->notifyReady();
    // \todo Can we have UserNumber increasing again?
  }

private:

  /// Notify the customer tasks this buffer generation is ready to use
  void notifyReady() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      ReadyToUse = true;
    }
    ReadyCV.notify_all();
  }

};


/** The abstraction to represent SYCL tasks executing inside command_group

    "enable_shared_from_this" allows to access the shared_ptr behind the
    scene.
 */
struct Task : std::enable_shared_from_this<Task>,
              public Debug<Task> {
  /// The buffers that are used by this task
  std::vector<std::shared_ptr<BufferCustomer>> Buffers;

  /// Add a new task to the task graph and schedule for execution
  void schedule(std::function<void(void)> F) {
    auto execution = [&] {
      // Wait for the required buffers to be ready
      acquireBuffers();
      // Execute the kernel
      F();
      // Release the required buffers for other uses
      releaseBuffers();
    };
#if TRISYCL_ASYNC
    /* If in asynchronous execution mode, execute the functor in a new
       thread */
    std::thread thread(execution);
    // std::cout << thread.get_id();
    // Detach the thread since it will synchronize by its own means
    thread.detach();
#else
    // Just a synchronous execution otherwise
    execution();
#endif
  }


  void acquireBuffers() {
    for (auto &b : Buffers)
      b->wait();
  }


  void releaseBuffers() {
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
    // Add the task as a new client for the buffer of the accessor
    //A.getBuffer().addClient(A, shared_from_this());
    Buffers.push_back(BufferCustomer::getBufferCustomer(A));
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
