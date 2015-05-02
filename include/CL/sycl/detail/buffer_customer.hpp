#ifndef TRISYCL_SYCL_BUFFER_CUSTOMER_HPP
#define TRISYCL_SYCL_BUFFER_CUSTOMER_HPP

/** \file The concept of buffer_customer behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "CL/sycl/detail/debug.hpp"

namespace cl {
namespace sycl {
namespace detail {

class BufferBase;
/// \todo is it needed?
class Task;

/** Keep track of the tasks waiting for the availability of a buffer
    generation, either to read it or to write it

    When we write into a buffer, we generate a new version of it (think
    "SSA")
*/
class BufferCustomer : public detail::debug<BufferCustomer> {
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


  /** Set the next generation of the buffer after this

      \todo Refactor this with an lock-free list?
  */
  void setNextGeneration(std::shared_ptr<BufferCustomer> BC) {
    nextGeneration = BC;
  }


  /// Add a new task as a customer of the buffer generation
  void add(std::shared_ptr<Task> task, bool writeAccess) {
    WriteAccess = writeAccess;
    UserNumber++;
    TRISYCL_DUMP_T("BufferCustomer::add() now UserNumber = " << UserNumber);
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
    TRISYCL_DUMP_T("BufferCustomer::release() now UserNumber = " << UserNumber);
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


  /** Wait for the release of the buffer generation before the host can
      use it
  */
  void waitReleased() {
    TRISYCL_DUMP_T("BufferCustomer::waitReleased() UserNumber = " << UserNumber);
    {
      std::unique_lock<std::mutex> UL { ReleasedMutex };
      ReleasedCV.wait(UL, [&] { return UserNumber == 0; });
    }
  }


  /// Notify the customer tasks this buffer generation is ready to use
  void notifyReady() {
    {
      std::unique_lock<std::mutex> UL { ReadyMutex };
      // \todo This lock can be avoided if ReadyToUse is atomic
      ReadyToUse = true;
    }
    TRISYCL_DUMP_T("BufferCustomer::notifyReady()");
    ReadyCV.notify_all();
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

#endif // TRISYCL_SYCL_BUFFER_CUSTOMER_HPP
