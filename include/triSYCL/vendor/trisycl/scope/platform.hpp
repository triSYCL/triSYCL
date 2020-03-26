#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_PLATFORM_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_PLATFORM_HPP

/** \file The SYCL conceptual platform with a platform scope storage


    \todo Add some caching or prevent multiple creation of storage on
    the platform ? Or is this a feature to have different scopes on
    the same platform?

    \todo The device and platform are very similar. Use a common
    mix-in to implement both of them?

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/platform.hpp"
#include "triSYCL/vendor/trisycl/scope/platform/detail/platform.hpp"

/// This is an extension providing scope storage for platforms
#define SYCL_VENDOR_TRISYCL_PLATFORM_SCOPE 1

namespace trisycl::vendor::trisycl::scope {

/** \addtogroup vendor_trisycl_scope triSYCL extension for storage scopes
    @{
*/

/// An empty platform scope to make compilation errors clearer
struct empty_platform_scope {
};


/// A conceptual platform implementing some platform-scoped storage
template <typename PlatformStorage = empty_platform_scope>
class platform
  /* Use the underlying platform implementation that can be shared in the
     SYCL model */
  : public
    ::trisycl::detail::shared_ptr_implementation<platform<PlatformStorage>,
                                                 detail::platform
                                                 <PlatformStorage>> {

  using spi =
    ::trisycl::detail::shared_ptr_implementation<platform<PlatformStorage>,
                                                 detail::platform
                                                 <PlatformStorage>>;

  // Allows the comparison operation to access the implementation
  friend spi;

public:

  // Make the implementation member directly accessible in this class
  using spi::implementation;

  /// The type of the scoped storage in the platform
  using storage_type = PlatformStorage;

  /// The platform scope storage type
  using platform_scope_type = PlatformStorage;

  /// True if there is some storage in the platform
  static constexpr bool has_some_storage_p =
    not std::is_same_v<PlatformStorage, empty_platform_scope>;

  /** Construct the platform with some platform-scoped storage on top
      of a SYCL platform

      \param[in] p is the real platform to use
  */
  platform(const ::trisycl::platform &p) :
    spi {
      new detail::platform<PlatformStorage> { p }
    } {}


  /// By default use the default (host) platform
  platform() : platform(::trisycl::platform {}) {}


  /// It is still copyable
  platform(const platform&) = default;


  /// It is still movable
  platform(platform&&) = default;


  /// Access to the platform-scoped storage
  auto& get_storage() const {
    return implementation->get_storage();
  }


  /** Add a conversion to \c trisycl::platform& so it can be used as
      a normal platform */
    operator ::trisycl::platform&() const {
      return implementation->get_underlying_platform();
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

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_SCOPE_PLATFORM_HPP
