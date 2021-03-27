#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_QUEUE_DETAIL_QUEUE_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_QUEUE_DETAIL_QUEUE_HPP

/** \file Implementation details of the SYCL conceptual queue with a
    queue scope storage

    \todo The device and queue are very similar. Use a common
    mix-in to implement both of them?

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/queue.hpp"

namespace trisycl::vendor::trisycl::scope::detail {

/** \addtogroup vendor_trisycl_scope triSYCL extension for storage scopes
    @{
*/

/** A conceptual queue implementing some queue-scoped storage
    constructed from a device with some device-scope storage
*/
template <typename Device,
          typename QueueStorage>
class queue {
  /// The storage-less queue behind the scene
  ::trisycl::queue q;

  /** The queue-scoped storage default-initialized

      \todo For now it is allocated on the host in this CPU emulation
      but a device compiler and run time can create this on a real
      device
  */
  QueueStorage scope_storage;

  /// The scoped device the scoped queue is built into
  Device device_with_scope;

public:

  /** Construct the queue with some queue-scoped storage on top
      of a SYCL queue

      \param[in] q is the real queue to use
  */
  queue(const ::trisycl::queue &q) : q { q } {}


  /** Construct the queue with some queue-scoped storage on top
      of a SYCL queue

      \param[in] q is the real queue to use
      \param[in] d is the a scoped device to use
  */
  queue(const ::trisycl::queue &q, const Device &d)
    : q { q }, device_with_scope { d } {}


  /// Get the queue behind the curtain
  auto get_underlying_queue() {
    return q;
  }


  /// Access to the queue-scoped storage
  auto& get_storage() {
    return scope_storage;
  }


  /// Access to the underlying scoped device
  auto& get_device() {
    return device_with_scope;
  }

};

/// @} to end the vendor_trisycl_scope Doxygen group

}


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_QUEUE_DETAIL_QUEUE_HPP
