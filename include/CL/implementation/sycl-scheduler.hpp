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
  // State when the buffer generation is ready to be used
  bool ReadyToUse;
  // To signal when it is ready
  std::condition_variable ReadyCV;
  /// To protect the access to the condition variable
  std::mutex ReadyMutex;
  // Count the number of accelerator-side usage of this buffer generation
  std::atomic<unsigned int> UserNumber;
  /** To signal when the buffer generation is no longer used from the
      accelerator side and can be used for example through a host
      accessor */
  std::condition_variable ReleasedCV;
  /// To protect the access to the condition variable
  std::mutex ReleasedMutex;

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


  // Wait for the latest generation of the buffer before the host can use it
   static void wait(BufferBase &B) {
     // If there is nobody using the buffer, no need to wait
     if (B.LastBufferCustomer)
       /* In a correct SYCL program there should be no more task creation
          using a buffer given to use by a hast accessor so this should be
          race free */
       B.LastBufferCustomer->waitReleased();
   }


  /// Add a new task as a customer of the buffer generation
  void add(std::shared_ptr<Task> task, bool writeAccess) {
    WriteAccess = writeAccess;
    UserNumber++;
  }


  /// Wait for the buffer generation to be ready to use by a kernel task
  void wait() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      ReadyCV.wait(UL, [&] { return ReadyToUse; });
    }
  }


  /// Release the buffer generation usage by a  kernel task
  void release() {
    UserNumber--;
    if (UserNumber == 0) {
      /* If there is no task using this generation of the buffer, first
         notify the host accessors waiting for it, if any */
      ReleasedCV.notify_all();

      /* And then make the next generation ready if any. Note that if the
         SYCL program is race condition-free, there should be no host
         accessor waiting for a generation which is not the last one...

         \todo: add some SYCL semantics runtime verification
      */
      if (nextGeneration)
        nextGeneration->notifyReady();
    }
    // \todo Can we have UserNumber increasing again?
  }


  // Wait for the release of the buffer generation before the host can use
  // it
  void waitReleased() {
    {
      std::unique_lock<std::mutex> UL { ReleasedMutex };
      ReleasedCV.wait(UL, [&] { return UserNumber == 0; });
    }
  }


private:

  /// Notify the customer tasks this buffer generation is ready to use
  void notifyReady() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      // \todo This lock can be avoided if ReadyToUse is atomic
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
    /** To keep a reference on the Task shared_ptr after the end of the
        command group, capture it by copy in the following lambda.  This
        should be easier in C++17 with move semantics on capture
    */
    auto task = shared_from_this();
    auto execution = [=] {
      // Wait for the required buffers to be ready
      task->acquireBuffers();
      // Execute the kernel
      TRISYCL_DUMP_T("Execute the kernel");
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
