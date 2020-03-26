#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DEVICE_DETAIL_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DEVICE_DETAIL_DEVICE_HPP

/** \file Implementation details of the SYCL conceptual device with a
    device scope storage

    \todo The device and platform are very similar. Use a common
    mix-in to implement both of them?

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/device.hpp"

namespace trisycl::vendor::trisycl::scope::detail {

/** \addtogroup vendor_trisycl_scope triSYCL extension for storage scopes
    @{
*/

/** A conceptual device implementing some device-scoped storage and
    based on a platform with platform-scoped storage
*/
template <typename DeviceStorage
          , typename ScopedPlatform>
class device {
  /// The storage-less device behind the scene
  ::trisycl::device d;

  /** The device-scoped storage default-initialized

      \todo For now it is allocated on the host in this CPU emulation
      but a device compiler and runtime can create this on a real
      device
  */
  DeviceStorage scope_storage;

  /// The scoped platform the scoped device is built into
  ScopedPlatform platform_with_scope;


public:

  /** Construct the device with some device-scoped storage on top
      of a SYCL device

      \param[in] d is the real device to use
  */
  device(const ::trisycl::device &d,
         const ScopedPlatform &p)
    : d { d }, platform_with_scope { p } {}


  /** Construct the device with some device-scoped storage on top
      of a SYCL device

      \param[in] d is the real device to use
  */
  device(const ::trisycl::device &d) : d { d } {}

  /// Get the device behind the curtain
  auto& get_underlying_device() {
    return d;
  }


  /// Access to the device-scoped storage
  auto& get_storage() {
    return scope_storage;
  }


  /// Access to the underlying scoped platform
  auto& get_platform() {
    return platform_with_scope;
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

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DEVICE_DETAIL_DEVICE_HPP
