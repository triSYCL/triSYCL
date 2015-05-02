#ifndef TRISYCL_SYCL_BUFFER_BASE_HPP
#define TRISYCL_SYCL_BUFFER_BASE_HPP

/** \file The buffer_base behind the buffers

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>
#include <mutex>

#include "CL/sycl/buffer/detail/buffer_customer.hpp"

namespace cl {
namespace sycl {
namespace detail {

template <typename T,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct AccessorImpl;

/** Factorize some template independent buffer aspects in a base class
 */
 struct BufferBase {
   /// If the data are read-only, store the information for later optimization.
   /// \todo Replace this by a static read-only type for the buffer
   bool ReadOnly;

   /// Store the BufferCustomer for the last generation of this buffer
   std::shared_ptr<BufferCustomer> LastBufferCustomer;
   std::mutex ProtectBuffer;


   BufferBase(bool ReadOnly) : ReadOnly { ReadOnly } {}


   /// Lock the BufferBase structure by returning a unique_lock on the mutex
   std::unique_lock<std::mutex> lock() {
     return std::unique_lock<std::mutex> { ProtectBuffer };
   }


   std::shared_ptr<BufferCustomer> getLastBufferCustomer() {
     return LastBufferCustomer;
   }


   void setLastBufferCustomer(std::shared_ptr<BufferCustomer> BC) {
     LastBufferCustomer = BC;
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
        OldBC->setNextGeneration(BC);
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
          using a buffer given to use by a host accessor so this should be
          race free */
       B.LastBufferCustomer->waitReleased();
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

#endif // TRISYCL_SYCL_BUFFER_BASE_HPP
