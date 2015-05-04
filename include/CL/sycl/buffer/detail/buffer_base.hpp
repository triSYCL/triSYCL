#ifndef TRISYCL_SYCL_BUFFER_BASE_HPP
#define TRISYCL_SYCL_BUFFER_BASE_HPP

/** \file The buffer_base behind the buffers

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <memory>
#include <mutex>

#include "CL/sycl/access.hpp"
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
 struct buffer_base {
   /// If the data are read-only, store the information for later optimization.
   /// \todo Replace this by a static read-only type for the buffer
   bool read_only;

   /// Store the buffer_customer for the last generation of this buffer
   std::shared_ptr<buffer_customer> last_buffer_customer;
   std::mutex protect_buffer;


   buffer_base(bool read_only) : read_only { read_only } {}


   /// Lock the buffer_base structure by returning a unique_lock on the mutex
   std::unique_lock<std::mutex> lock() {
     return std::unique_lock<std::mutex> { protect_buffer };
   }


   std::shared_ptr<buffer_customer> get_last_buffer_customer() {
     return last_buffer_customer;
   }


   void set_last_buffer_customer(std::shared_ptr<buffer_customer> bc) {
     last_buffer_customer = bc;
   }

  /// Get the buffer customer associated to the latest version of the buffer
  template <typename T,
            std::size_t dimensions,
            access::mode mode,
            access::target target = access::global_buffer>
  static std::shared_ptr<buffer_customer>
  get_buffer_customer(AccessorImpl<T, dimensions, mode, target> &a) {
    buffer_base &b = a.get_buffer();
    {
      /// Use atomic list?
      // Protect the update of last_buffer_customer in the Buffer
      auto lock = b.lock();
      std::shared_ptr<buffer_customer> bc = b.get_last_buffer_customer();
      auto old_bc = bc;
      /* When we write into a buffer, we generate a new version of it (think
         "SSA"). Of course we do it also when there is not yet any
         buffer_customer */
      if (!bc || a.is_write_access()) {
        bc = std::make_shared<buffer_customer>(b, a.is_write_access());
        b.set_last_buffer_customer(bc);
      }

      if (old_bc)
        // \todo Use atomic list instead
        old_bc->set_next_generation(bc);
      else
        // If we just created the buffer_customer, it is ready to use
        bc->notify_ready();

      return bc;
    }
  }


  // Wait for the latest generation of the buffer before the host can use it
   static void wait(buffer_base &b) {
     // If there is nobody using the buffer, no need to wait
     if (b.last_buffer_customer)
       /* In a correct SYCL program there should be no more task creation
          using a buffer given to use by a host accessor so this should be
          race free */
       b.last_buffer_customer->wait_released();
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
