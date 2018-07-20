#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LOCK_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LOCK_HPP

/** \file The lock mechanism used by some MathEngine tiles

    Note that this MathEngine concept is not a pure lock, but more
    like a lock associated with a conditional variable, to follow the
    C++ jargon.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include <condition_variable>
#include <memory>
#include <mutex>


namespace cl::sycl::vendor::xilinx::acap::me {

/** The lock infrastructure used by MathEngine memory modules and shim tiles

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    4.7 Lock Unit, p. 129
*/
struct lock_unit {
  /// The individual locking system
  struct locking_device {
    /* The problem here is that std::mutex and std::condition_variable
       are not moveable while the instantiation of a memory module uses
       move assignment with Boost.Hana...

       So allocate them dynamically and keep them in a std::unique_ptr
       so globally the type is moveable */

    /// The mutex to provide the basic protection mechanism
    std::unique_ptr<std::mutex> m { new std::mutex { } };
    /// The condition variable to wait/notify for some value
    std::unique_ptr<std::condition_variable> cv {
      new std::condition_variable { } };
    /// The value to be waited for, initialized to false on reset
    bool value = false;

    /// Lock the mutex
    void lock() {
      m->lock();
    }


    /// Unlock the mutex
    void unlock() {
      m->unlock();
    }


    /// Wait until the value is the expectation
    void wait_value(bool expectation) {
      std::unique_lock lk { *m };
      cv->wait(lk, [=]{ return expectation == value; });
    }


    /// Release the value with the expectation
    void release_value(bool expectation) {
      {
        std::unique_lock lk { *m };
        value = expectation;
      }
      cv->notify_one();
    }
  };

  locking_device locks[16];
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LOCK_HPP
