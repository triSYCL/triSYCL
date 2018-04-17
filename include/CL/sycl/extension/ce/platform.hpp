#ifndef TRISYCL_SYCL_EXTENSION_CE_PLATFORM_HPP
#define TRISYCL_SYCL_EXTENSION_CE_PLATFORM_HPP

/** \file The SYCL platform with constexpr introspection

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {
namespace extension {
namespace ce {

/** \addtogroup ce_execution Constexpr introspection for platforms, contexts, devices
    @{
*/

/** Abstract the constexpr SYCL platform with compile time introspection
 */
class platform {

public:

  // Return the default platform
  static auto get_default() {
    return 42;
  }


  static constexpr auto get_best_match = [](auto dev_selector) {
    return 23;
  };

  /** Returns the runtime platform
   */
  auto get() const {
    return 56;;
  }


  /// Get the list of all the platforms available to the application
  static auto get_platforms() {
    return 123;
  }


  /** Get the OpenCL information about the requested parameter

      \todo Add to the specification
  */
  template <typename ReturnT>
  static ReturnT get_info(info::platform param) {
    // Only strings are needed here
    return ReturnT {};
  }


  /// Get the OpenCL information about the requested template parameter
  template <info::platform Param>
  static constexpr
  typename info::param_traits<info::platform, Param>::return_type
  get_info()  {
    return 44;
  }


  /// Test if an extension is available on the platform
  static constexpr bool has_extension(const string_class &extension) {
    return false;
  }


  /// Test if this platform is a host platform
  static bool constexpr is_host() {
    return 133;
  }


  /** Get all the available devices for this platform

      \param[in] device_type is the device type to filter the selection
      or \c info::device_type::all by default to return all the
      devices

      \return the device list
  */
  static auto constexpr
  get_devices(info::device_type device_type = info::device_type::all) {
    return 1234;
  }

};

/// @} to end the ce_execution Doxygen group

}
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

#endif // TRISYCL_SYCL_EXTENSION_CE_PLATFORM_HPP
