#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DEVICE_HPP

/** \file The SYCL conceptual device with a device scope storage


    \todo Add some caching or prevent multiple creation of storage on
    the device ? Or is this a feature to have different scopes on
    the same device?

    \todo The device and platform are very similar. Use a common
    mix-in to implement both of them?

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/device.hpp"
#include "CL/sycl/vendor/trisycl/scope/device/detail/device.hpp"
#include "CL/sycl/vendor/trisycl/scope/platform.hpp"

/// This is an extension providing scope storage for devices
#define SYCL_VENDOR_TRISYCL_DEVICE_SCOPE 1

namespace cl::sycl::vendor::trisycl::scope {

/** \addtogroup vendor_trisycl_scope triSYCL extension for
    storage scopes
    @{
*/

/// An empty device scope to make compilation errors clearer
struct empty_device_scope {
};


/** A conceptual device implementing some device-scoped storage and
    based on a platform with platform-scoped storage
*/
template <typename DeviceStorage
          , typename ScopedPlatform =
          cl::sycl::vendor::trisycl::scope::empty_platform_scope
          >
class device
  /* Use the underlying device implementation that can be shared in the
     SYCL model */
  : public
    cl::sycl::detail::shared_ptr_implementation<device<DeviceStorage,
                                                       ScopedPlatform>,
                                                detail::device
                                                  <DeviceStorage,
                                                   ScopedPlatform>> {
  using spi =
    cl::sycl::detail::shared_ptr_implementation<device<DeviceStorage,
                                                       ScopedPlatform>,
                                                detail::device
                                                  <DeviceStorage,
                                                   ScopedPlatform>>;

  // Allows the comparison operation to access the implementation
  friend spi;

public:

  // Make the implementation member directly accessible in this class
  using spi::implementation;

  /// The type of the scoped storage in the device
  using storage_type = DeviceStorage;

  /// True if there is some storage in the device
  static constexpr bool has_some_storage_p =
    not std::is_same_v<DeviceStorage, empty_device_scope>;


  /** Construct the device with some device-scoped storage on top
      of a SYCL device

      \param[in] d is the real device to use
  */
  device(const cl::sycl::device &d,
         const ScopedPlatform &p = {}) :
    spi {
      new detail::device<DeviceStorage, ScopedPlatform> { d, p }
    } {}


  /// By default use the default (host) device with no platform storage
  device() : device(cl::sycl::device {}) {}


  /// It is still copyable
  device(const device&) = default;


  /// It is still movable
  device(device&&) = default;


  /// Access to the device-scoped storage
  auto& get_storage() const {
    return implementation->get_storage();
  }


  /// Access to the underlying scoped platform
  auto& get_platform() const {
    return implementation->get_platform();
  }


  /** Add a conversion to \c cl::sycl::device& so it can be used as
      a normal device */
    operator cl::sycl::device&() const {
      return implementation->get_underlying_device();
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

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_DEVICE_HPP
