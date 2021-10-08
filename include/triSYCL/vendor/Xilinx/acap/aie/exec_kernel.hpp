#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_EXEC_KERNEL_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_EXEC_KERNEL_HPP

#ifndef __SYCL_XILINX_AIE__
#error "This header should only be included when targeting hardware"
#endif

/** \file

    This file implements the kernel outlining and execution logic when executing
    on hardware

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef __SYCL_DEVICE_ONLY__
#include "device_allocator.hpp"
#include "log.hpp"
#endif

#include "xaie_wrapper.hpp"
#include <string>

#include "triSYCL/detail/program_manager.hpp"

namespace trisycl::vendor::xilinx::acap::aie::detail {

template <typename TileHandle> struct exec_kernel {
#ifdef __SYCL_DEVICE_ONLY__
  /// Trigger the outlining on device with __attribute__((sycl_kernel))
  template <typename KernelName, typename KernelType>
  __attribute__((sycl_kernel)) void kernel_outliner(KernelType &k) {
    k();
  }
#else
  template <typename KernelName, typename KernelType>
  void kernel_outliner(KernelType &k) {
    k();
  }
#endif

#ifdef __SYCL_DEVICE_ONLY__
  /// This will be invoked on device before any user code.
  static void kernel_prerun() { acap::heap::init_allocator(); }
  /// This will be invoked on device when exiting normally after any user code.
  static void kernel_postrun() {
    acap::heap::assert_no_leak();
    finish_kernel();
  }
#else
  static void kernel_prerun() {}
  static void kernel_postrun() {}
#endif

  /// kernel_builder will insert prerun and postrun, and get an instance of the
  /// kernel object and the tile handle.
  /// In the current implementation the opencl-like kernel built by the device
  /// compiler is not used. the only thing that the compiler does is call one of
  /// the lambdas built by kernel_builder
  template <typename KernelType>
  auto kernel_builder(KernelType &) requires requires(KernelType k) {
    k();
  }
  {
    return []() mutable {
#ifdef __SYCL_DEVICE_ONLY__
      KernelType *k = (KernelType *)(hw::self_tile_addr(hw::get_parity_dev()) +
                                     hw::args_begin_offset);
      kernel_prerun();
      k->operator()();
      kernel_postrun();
#endif
    };
  };
  template <typename KernelType, typename Param>
  auto kernel_builder(KernelType &) requires requires(KernelType k) {
    k(*this);
  }
  {
    return []() mutable {
#ifdef __SYCL_DEVICE_ONLY__
      KernelType *k = (KernelType *)(hw::self_tile_addr(hw::get_parity_dev()) +
                                     hw::args_begin_offset);
      kernel_prerun();
      /// TODO TileHandle should be properly initialized.
      TileHandle th;
      k->operator()(th);
      kernel_postrun();
#endif
    };
  }
  template <typename KernelType>
  auto kernel_builder(KernelType &) requires requires(KernelType k) {
    k.run();
  }
  {
    return []() mutable {
#ifdef __SYCL_DEVICE_ONLY__
      KernelType *k = (KernelType *)(hw::self_tile_addr(hw::get_parity_dev()) +
                                     hw::args_begin_offset);
      kernel_prerun();
      k->run();
      kernel_postrun();
#endif
    };
  }
  template <typename KernelType>
  auto kernel_builder(KernelType &) requires requires(KernelType k) {
    k.run(*this);
  }
  {
    return []() mutable {
#ifdef __SYCL_DEVICE_ONLY__
      KernelType *k = (KernelType *)(hw::self_tile_addr(hw::get_parity_dev()) +
                                     hw::args_begin_offset);
      kernel_prerun();
      /// TODO TileHandle should be properly initialized.
      TileHandle th;
      k->run(th);
      kernel_postrun();
#endif
    };
  }

  template <typename K> void exec(xaie::handle dev_handle, K k) {
    acap::hw::position pos = xaie_pos_to_acap_pos(dev_handle.tile);

    /// The host and device must see the same kernel type so we need to build
    /// the kernel here.
    auto Kernel = kernel_builder(k);

#ifdef __SYCL_DEVICE_ONLY__
    /// On device trigger outlining of device code.
    kernel_outliner<typename std::decay<decltype(Kernel)>::type>(Kernel);

#else
    /// Host side

    // The name is captured by it's non-reference type and has to be in
    // the cl::sycl::detail namespace as the integration header is
    // defined to be in this namespace (and all our implementation
    // resides in trisycl by default, so ::detail resolves to
    // trisycl::detail)
    std::string kernelName = ::trisycl::detail::KernelInfo<
        typename std::decay<decltype(Kernel)>::type>::getName();

    /// The sycl-chess script will build 2 version per kernels one with west
    /// parity one with east parity.
    if (hw::get_parity(pos) == hw::parity::west)
      kernelName += "_west";
    else
      kernelName += "_east";

    auto kernelImage =
        ::trisycl::detail::program_manager::instance()->get_image(kernelName);

#ifdef TRISYCL_DEBUG_IMAGE
    // Image Dump using name retrieval for Debug, separate debug define
    // as dumping 400 images when at maximum array capacity is not
    // necessarily something you always want to do when debugging.
    //
    // This differentiates from the program_manager image dump in that
    // it helps check whether the names are correctly correlating to the
    // correct ELF images and if there is some breakage in the storing
    // of the images.
    detail::program_manager::instance()->image_dump(
        kernelName, "run_aie_" + kernelName + ".elf");
#endif
    {
      // auto Transaction = dev_handle.get_transaction();
      dev_handle.core_reset();

      TRISYCL_DUMP2("Loading Kernel " << kernelName << " ELF to tile (" << pos.x
                                      << ',' << pos.y << ")",
                    "exec");

      dev_handle.load_elf_image(kernelImage);

      TRISYCL_DUMP2("Loaded Kernel " << kernelName << " ELF to tile (" << pos.x
                                     << ',' << pos.y
                                     << ") beginning tile execution",
                    "exec");

      if constexpr (requires { k.prerun(); })
        k.prerun();

      TRISYCL_DUMP2("Starting AIE tile ("
                        << pos.x << ',' << pos.y << ") "
                        << "Associated Tile Kernel Name: " << kernelName
                        << "- beginning prerun execution",
                    "exec");

      dev_handle.core_run();
    }
#endif
  }
};
} // namespace trisycl::vendor::xilinx::acap::aie::detail

#endif
