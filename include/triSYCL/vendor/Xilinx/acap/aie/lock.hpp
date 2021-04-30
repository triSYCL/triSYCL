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
#if defined(__SYCL_DEVICE_ONLY__)
#include "acap-intrinsic.h"
#else
#include "xaie_wrapper.hpp"
#endif
#else
#include <condition_variable>
#include <mutex>
#endif

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

struct device_lock {
#if defined(__SYCL_XILINX_AIE__)
#if defined(__SYCL_DEVICE_ONLY__)
/// on device acap
  int id;

  /// Lock the mutex
  void acquire() { acap_intr::acquire(id); }

  /// Unlock the mutex
  void release() { acap_intr::release(id); }

  /// Wait until the internal value has the val
  void acquire_with_value(bool val) { acap_intr::acquire(id, val); }

  /// Release and update with a new internal value
  void release_with_value(bool val) { acap_intr::release(id, val); }

#else
  
  int id;

  xaie::XAie_DevInst *dev_inst;
  xaie::XAie_LocType loc;

  void acquire() {
    TRISYCL_DUMP2("acquiring lock (" << (int)loc.Col << ", " << (int)loc.Row - 1
                                     << ") id: " << (int)id,
                  "sync");
    TRISYCL_XAIE(XAie_LockAcquire(
        dev_inst, loc, xaie::XAie_LockInit(id, XAIE_LOCK_WITH_NO_VALUE),
        0xffffffff));
    TRISYCL_DUMP2("", "done");
  }

  /// Unlock the mutex
  void release() {
    TRISYCL_DUMP2("releasing lock (" << (int)loc.Col << ", " << (int)loc.Row - 1
                                     << ") id: " << (int)id,
                  "sync");
    TRISYCL_XAIE(XAie_LockRelease(
        dev_inst, loc, xaie::XAie_LockInit(id, XAIE_LOCK_WITH_NO_VALUE),
        0xffffffff));
    TRISYCL_DUMP2("", "done");
  }

  /// Wait until the internal value has the val
  void acquire_with_value(bool val) {
    TRISYCL_DUMP2("acquiring lock (" << (int)loc.Col << ", " << (int)loc.Row - 1
                                     << ") id: " << (int)id << "val: " << val,
                  "sync");
    XAie_LockAcquire(dev_inst, loc, xaie::XAie_LockInit(id, val), 0xffffffff);
    TRISYCL_DUMP2("", "done");
  }

  /// Release and update with a new internal value
  void release_with_value(bool val) {
    TRISYCL_DUMP2("releasing lock (" << (int)loc.Col << ", " << (int)loc.Row - 1
                                     << ") id: " << (int)id << "val: " << val,
                  "sync");
    XAie_LockRelease(dev_inst, loc, xaie::XAie_LockInit(id, val), 0xffffffff);
    TRISYCL_DUMP2("", "done");
  }

#endif
#else
#error "not yet supported"
  /// in CPU emulation
  /// The mutex to provide the basic protection mechanism
  std::mutex m{};

  /// The condition variable to wait/notify for some value
  std::condition_variable cv{};

  /// The value to be waited for, initialized to false on reset
  bool value = false;
    /// Lock the mutex
  void acquire() { m.lock(); }

  /// Unlock the mutex
  void release() { m.unlock(); }

  /// Wait until the internal value has the val
  void acquire_with_value(bool val) {
    std::unique_lock lk{m};
    cv.wait(lk, [=] { return val == value; });
  }

  /// Release and update with a new internal value
  void release_with_value(bool val) {
    {
      std::unique_lock lk{m};
      value = val;
    }
    // By construction there should be only one client waiting for it
    cv.notify_one();
  }
#endif
};

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
