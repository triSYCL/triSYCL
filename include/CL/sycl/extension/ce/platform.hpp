#ifndef TRISYCL_SYCL_EXTENSION_CE_PLATFORM_HPP
#define TRISYCL_SYCL_EXTENSION_CE_PLATFORM_HPP

/** \file The SYCL platform with constexpr introspection

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/hana.hpp>

namespace cl {
namespace sycl {
namespace extension {
namespace ce {

namespace bh = boost::hana;
using namespace std::literals;

/** \addtogroup ce_execution Constexpr introspection for platforms, contexts, devices
    @{
*/

auto find_platform = [] (auto&& tuple, auto const& info_pair) {
  // For each platform
  return bh::find_if(tuple, [&info_pair](const auto &p) {
      // For each information of the platform
      return bh::bool_c<bh::contains(bh::second(p), info_pair)>;
      //return bh::true_c;//bh::contains(bh::second(p), info_pair);
    });
};


/** Abstract the constexpr SYCL platform with compile time introspection
 */
class platform {

  /// Assume the platform name is unique and so use it as a key
  const char * name;

  /// The architecture of the available platforms at compile time
  static constexpr auto architecture =
    bh::make_tuple(
      bh::make_pair(
          bh::type_c<cl::sycl::host_selector>
        , bh::make_tuple(
            [] { return cl::sycl::platform {}; }
            // \todo The following information should be used in
            // include/CL/sycl/platform/detail/host_platform.hpp too
          , bh::make_tuple(
                  bh::make_pair(info::platform::profile,
                                "FULL_PROFILE")
                , bh::make_pair(info::platform::version,
                                "1.2")
                , bh::make_pair(info::platform::name,
                                "triSYCL host platform")
                , bh::make_pair(info::platform::vendor,
                                "triSYCL Open Source project")
                , bh::make_pair(info::platform::extensions,
                                detail::host_platform::platform_extensions)
                           )
                         )
                    )
    , bh::make_pair(
          bh::type_c<cl::sycl::cpu_selector>
        , bh::make_tuple(
            [] { return cl::sycl::platform { cl::sycl::cpu_selector {} }; }
          , bh::make_tuple(
                  bh::make_pair(info::platform::profile,
                                "FULL_PROFILE")
                , bh::make_pair(info::platform::version,
                                "OpenCL 1.2 pocl 1.2-pre Debug+Asserts, LLVM 7.0.0svn, SPIR, SLEEF, POCL_DEBUG")
                , bh::make_pair(info::platform::name,
                                "Portable Computing Language")
                , bh::make_pair(info::platform::vendor,
                                "The pocl project")
                , bh::make_pair(info::platform::extensions,
                                "cl_khr_byte_addressable_store cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_3d_image_writes cl_khr_spir cl_khr_fp64 cl_khr_int64_base_atomics cl_khr_int64_extended_atomics cl_khr_fp64")
                           )
                         )
                    )
                   );

public:

  static auto constexpr find() {
    return find_platform(architecture,
                         bh::make_pair(info::platform::name,
                                       "Portable Computing Language"));
    /*
    return bh::contains(architecture, bh::make_pair(info::platform::name,
                                                    "Portable Computing Language"));
    */
  }


  constexpr platform()
  : platform { "host" }
  {}


  constexpr platform(const char * name)
  : name { name }
  {}


  auto constexpr get_name() const {
    return name;
  }


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
    return bh::make_tuple(platform { "host" },
                                   platform { "pocl" },
                                   platform { "xilinx" });
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
