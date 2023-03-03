#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LOCK_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LOCK_HPP

/** \file

    The lock mechanism used by some AI Engine tiles

    Note that this AI Engine concept is not a pure lock, but more
    like a lock associated with a conditional variable, to follow the
    C++ jargon.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#if defined(__SYCL_XILINX_AIE__)
/// Use hardware intrinsics on real device
#include "acap-intrinsic.h"

/// Use libxaiengine on host with real hardware
#include "xaie_wrapper.hpp"
#include <thread>

#else

/// In CPU emulation, use fiber synchronization primitives
#include <memory>
#include <mutex>

#include <boost/fiber/all.hpp>
#endif

#include "triSYCL/detail/enum.hpp"
#include "hardware.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/// This is a software implementation of a barrier for synchronization between 1
/// tile and the host.
struct soft_barrier {
#if defined(__SYCL_XILINX_AIE__)
  enum {
    device = 0,
    host = 1,
  };

#if !defined(__SYCL_DEVICE_ONLY__)
  struct host_side {
    xaie::handle h;
    uint32_t dev_offset;
    /// Wait for the device to reach the barrier.
    void wait() {
      detail::no_log_in_this_scope nls;
      uint32_t counter =
          h.mem_read(dev_offset + offsetof(device_side, counters[host]));
      /// Wait for the device to arrive.
      while (h.mem_read(dev_offset + offsetof(device_side, counters[device])) ==
             counter)
        std::this_thread::yield();
      // Notify the device that the host has arrived too
      counter++;
      h.mem_write(dev_offset + offsetof(device_side, counters[host]), counter);
    }
    /// Return true if the device was waiting and perform the barrier.
    /// Return false if the device was not waiting and doesn't perform the
    /// barrier.
    bool try_arrive() {
      detail::no_log_in_this_scope nls;
      uint32_t counter =
          h.mem_read(dev_offset + offsetof(device_side, counters[host]));
      if (h.mem_read(dev_offset + offsetof(device_side, counters[device])) ==
          counter)
        // If the device has not arrived, the host does not arrive either
        return false;
      // Notify the device that the host has arrived too
      counter++;
      h.mem_write(dev_offset + offsetof(device_side, counters[host]), counter);
      return true;
    }
  };
#endif
  class device_side {
#if !defined(__SYCL_DEVICE_ONLY__)
    /// Allow host_side to use offsetof on members
    friend struct soft_barrier::host_side;
#endif
    /// This is volatile because it is used by the host and device concurrently
    /// so the value may spuriously change and cannot be predicted by the
    /// compiler. counters[device] will be written to by the device and read by
    /// the host and counters[host] will be written to by the host and read by
    /// the device.
    volatile uint32_t counters[2];

  public:
#if defined(__SYCL_DEVICE_ONLY__)
    /// The device_side cannot be moved on device because the host_side depends
    /// on its address. The device_side should be initialized to 0 from the host
    /// before the kernel starts.
    device_side() = delete;
    device_side(const device_side &) = delete;
#endif
    /// Wait for the host.
    void wait() volatile {
      /// Prevent memory operation from being moved below the barrier
      acap_intr::memory_fence();
      /// Notify the host that the device has arrived
      counters[device] = counters[device] + 1;
      /// Wait for the host to arrive.
      while (counters[host] != counters[device]) {
        /// counters[host] and counters[device] are volatile so this loop is not
        /// dead.
      }
      /// Prevent memory operation from being moved above the barrier
      acap_intr::memory_fence();
    }
  };
#endif
};

/// This is a software implementation of a mutex for synchronization between
/// device and host. it is based on Dekker's algorithm



/// THIS CLASS IS STILL UNTESTED



struct soft_mutex {
#if defined(__SYCL_XILINX_AIE__)
  enum {
    device = 0,
    host = 1,
  };

#if !defined(__SYCL_DEVICE_ONLY__)
  /// This is a handle from the host to a lock on the device
  struct host_side {
    /// The handle of the device.
    xaie::handle h;
    /// The address of the soft_mutex::device_side on device.
    uint32_t dev_offset;

    void lock() {
      h.mem_write(dev_offset + offsetof(device_side, want_to_enter[host]), 1);
      while (
          h.mem_read(dev_offset + offsetof(device_side, want_to_enter[device]))) {
        if (h.mem_read(dev_offset + offsetof(device_side, turn)) != device) {
          h.mem_write(dev_offset + offsetof(device_side, want_to_enter[host]), 0);
          while (h.mem_read(dev_offset + offsetof(device_side, turn)) != device)
            /// I am not sure if it is a good idea to preempt our thread while
            /// waiting
            std::this_thread::yield();
          h.mem_write(dev_offset + offsetof(device_side, want_to_enter[host]), 1);
        }
      }
    }
    void unlock() {
      h.mem_write(dev_offset + offsetof(device_side, turn), 1);
      h.mem_write(dev_offset + offsetof(device_side, want_to_enter[host]), 0);
    }
  };
#endif

  /// This has the ownership semantics of a mutex on the device,  cannot be
  /// moved or copied. but on the host it is just an empty shell.
  class device_side {

#if !defined(__SYCL_DEVICE_ONLY__)
    /// allows host_side to used offset of on membres.
    friend struct soft_mutex::host_side;
#endif
    /// each member here only holds 2 states. but they cannot be less than 4
    /// byte because the host can only access with a 4 byte granularity. and we
    /// want the host be be able to do a simple unique memory access to modify
    /// any of these.
    volatile uint32_t want_to_enter[2];
    volatile uint32_t turn;

  public:
#if defined(__SYCL_DEVICE_ONLY__)
    /// The device_side cannot be moved on device because the host_side depends
    /// on its address. The device_side should be initialized from the host
    /// before the kernel starts.
    device_side() = delete;
    device_side(const device_side &) = delete;

    void lock() volatile {
      /// every memory access here is volatile.
      want_to_enter[device] = 1;
      while (want_to_enter[host]) {
        if (turn != device) {
          want_to_enter[device] = 0;
          while (turn != device)
            ;
          want_to_enter[device] = true;
        }
      }
      /// Prevent hardware reordering
      acap_intr::memory_fence();
      acap_intr::separator_scheduler();
    }
    void unlock() volatile {
      /// Prevent hardware reordering
      acap_intr::separator_scheduler();
      acap_intr::memory_fence();
      /// every memory access here is volatile.
      turn = host;
      want_to_enter[device] = 0;
    }
#endif
  };
#endif
};

#if defined(__SYCL_XILINX_AIE__)
struct hw_lock {
#if defined(__SYCL_DEVICE_ONLY__)
/// On ACAP device
hw_lock(hw::dir d, int i) : id{(detail::underlying_value(d) * 16) + i} {}
int id;

/// Lock the mutex
void acquire() { acap_intr::acquire(id); }

/// Release the lock
void release() { acap_intr::release(id); }

/// Wait until the internal value has the val
void acquire_with_value(bool val) { acap_intr::acquire(id, val); }

/// Update and release with a new internal value
void release_with_value(bool val) { acap_intr::release(id, val); }

#else
/// On host for real ACAP
  int id;

  xaie::handle h;

  /// Lock the lock
  void acquire() {
    h.acquire(id);
  }

  /// Unlock the lock
  void release() {
    h.release(id);
  }

  /// Wait until the internal value has the val
  void acquire_with_value(bool val) {
    h.acquire_with_value(id, val);
  }

  /// Release and update with a new internal value
  void release_with_value(bool val) {
    h.release_with_value(id, val);
  }

#endif
};
#else
/// in CPU emulation on both host and device.

/** The lock infrastructure used by AI Engine memory modules and shim tiles

    Based on Math Engine (ME) Architecture Specification, Revision v1.5
    June 2018

    4.4.6 Lock Interface, p. 115

    4.7 Lock Unit, p. 129
*/
struct lock_unit {
  // There are 16 hardware locks per memory tile
  auto static constexpr lock_number = 16;

  /// The individual locking system
  struct locking_device {
    /* The problem here is that \c mutex and \c condition_variable
       are not moveable while the instantiation of a memory module uses
       move assignment with Boost.Hana...

       So allocate them dynamically and keep them in a \c std::unique_ptr
       so globally the type is moveable */

    /// The mutex to provide the basic protection mechanism
    std::unique_ptr<boost::fibers::mutex> m { new boost::fibers::mutex { } };

    /// The condition variable to wait/notify for some value
    std::unique_ptr<boost::fibers::condition_variable> cv {
      new boost::fibers::condition_variable { } };

    /// The value to be waited for, initialized to false on reset
    bool value = false;

    /// Lock the mutex
    void acquire() {
      m->lock();
    }

    /// Release the lock
    void release() {
      m->unlock();
    }


    /// Wait until the internal value has the expectation
    void acquire_with_value(bool expectation) {
      std::unique_lock lk { *m };
      cv->wait(lk, [&] { return expectation == value; });
    }


    /// Release and update with a new internal value
    void release_with_value(bool new_value) {
      {
        std::unique_lock lk { *m };
        value = new_value;
      }
      // By construction there should be only one client waiting for it
      cv->notify_one();
    }
  };

  /// The locking units of the locking device
  locking_device locks[lock_number];

  /// Get the requested lock
  auto &lock(int i) {
    assert(0 <= i && i < lock_number);
    return locks[i];
  }
};

#endif
/// @} End the aie Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LOCK_HPP
