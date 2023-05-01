#ifndef AIE_DETAIL_SYNC_HPP
#define AIE_DETAIL_SYNC_HPP

#include <cstdint>
#include <functional>
#include <variant>
#include "hardware.hpp"
#include "xaie_wrapper.hpp"
#ifndef __SYCL_DEVICE_ONLY__
#include <iostream>
#endif

namespace aie::detail {

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
      trisycl::detail::no_log_in_this_scope nls;
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
      trisycl::detail::no_log_in_this_scope nls;
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
    __attribute__((noinline)) void wait() volatile {
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

}
#endif
