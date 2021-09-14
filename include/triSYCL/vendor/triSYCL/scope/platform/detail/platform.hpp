#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_PLATFORM_DETAIL_PLATFORM_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_PLATFORM_DETAIL_PLATFORM_HPP

/** \file Implementation details of the SYCL conceptual platform with a
    platform scope storage

    \todo The device and platform are very similar. Use a common
    mix-in to implement both of them?

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/platform.hpp"

namespace trisycl::vendor::trisycl::scope::detail {

/** \addtogroup vendor_trisycl_scope triSYCL extension for storage scopes
    @{
*/

/// A conceptual platform implementing some platform-scoped storage
template <typename PlatformStorage>
class platform {
  /// The storage-less platform behind the scene
  ::trisycl::platform p;

  /** The platform-scoped storage default-initialized

      \todo For now it is allocated on the host in this CPU emulation
      but a device compiler and run time can create this on a real
      device
  */
  PlatformStorage scope_storage;

public:

  /** Construct the platform with some platform-scoped storage on top
      of a SYCL platform

      \param[in] p is the real platform to use
  */
  platform(const ::trisycl::platform &p) : p { p } {}

  /// Get the platform behind the curtain
  auto get_underlying_platform() {
    return p;
  }


  /// Access to the platform-scoped storage
  auto& get_storage() {
    return scope_storage;
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

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_PLATFORM_DETAIL_PLATFORM_HPP
