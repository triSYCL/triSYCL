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

#include "device_allocator.hpp"
#include "hardware.hpp"
#include "log.hpp"
#include "xaie_wrapper.hpp"

#include <string>

#include "triSYCL/detail/program_manager.hpp"


#ifdef __SYCL_DEVICE_ONLY__

/// This declaration will invoke the constructors of every global variable in
/// the file.
extern "C" void __cxx_global_var_ctor() {}

/// This will be filled by our compiler with every destructor.
extern "C" void __cxx_global_var_dtor() {}

#endif

namespace trisycl::vendor::xilinx::acap::aie::detail {

/// Trigger the outlining on device with __attribute__((sycl_kernel))
template <typename KernelName, typename KernelType>
__attribute__((sycl_kernel)) void kernel_outliner(KernelType k) {
#ifdef __SYCL_DEVICE_ONLY__
  k();
#else
  (void)k;
#endif
}

template <typename TileHandle> struct exec_kernel {

#ifdef __SYCL_DEVICE_ONLY__
  /// This will be invoked on device before any user code.
  static void kernel_prerun() {
    __cxx_global_var_ctor();
  }
  /// This will be invoked on device when exiting normally after any user code.
  static void kernel_postrun() {
    __cxx_global_var_dtor();
    acap::heap::assert_no_leak();
    finish_kernel();
  }
  /// Generic kernel invoker on device.
  template <typename KernelType, typename Func>
  static void kernel_run(Func func) {
    /// This static variable provides storage on device for the capture of the
    /// kernel lambda. asm("kernel_lambda_capture") means that the compiler will
    /// emit the variable under the symbol kernel_lambda_capture. This
    /// variable's visibility is changed by the device compiler such that the
    /// symbol is visible
    static ::trisycl::detail::storage<KernelType> lambda_storage asm(
        "kernel_lambda_capture");
    kernel_prerun();
    func(lambda_storage.get());
    kernel_postrun();
  }

#else
  template <typename KernelType, typename Func>
  static void kernel_run(Func func) {}
#endif

  /// kernel_builder will insert prerun and postrun, and get an instance of the
  /// kernel object and the tile handle.
  /// In the current implementation the OpenCL-like kernel built by the device
  /// compiler is not used. The only thing that the compiler does is call one of
  /// the lambdas built by kernel_builder
  template <typename KernelType>
  auto kernel_builder(KernelType& k) requires requires(KernelType k) {
    k();
  }
  {
    return [] { kernel_run<KernelType>([](auto& k) { k(); }); };
  };
  template <typename KernelType>
  auto kernel_builder(KernelType& k) requires requires(KernelType k,
                                                       TileHandle t) {
    k(t);
  }
  {
    return [] {
      kernel_run<KernelType>([](auto& k) {
        TileHandle t {};
        k(t);
      });
    };
  }
  template <typename KernelType>
  auto kernel_builder(KernelType& k) requires requires(KernelType k) {
    k.run();
  }
  {
    return [] { kernel_run<KernelType>([](auto& k) { k.run(); }); };
  }
  template <typename KernelType>
  auto kernel_builder(KernelType& k) requires requires(KernelType k,
                                                       TileHandle t) {
    k.run(t);
  }
  {
    return [] {
      kernel_run<KernelType>([](auto& k) {
        TileHandle t {};
        k.run(t);
      });
    };
  }

  /// An empty placeholder we use as a default when there is no automatic
  /// paramater passing.
  struct ParamHandlerDefault {
    template<typename..., typename...Ts>
    void write_lambda(Ts&&...) {}
  };

  template <typename K, typename ParamHandler = ParamHandlerDefault>
  void exec(xaie::handle dev_handle, K k, uint32_t mem_tile_size = 0,
            ParamHandler* Phandler = nullptr) {
    acap::hw::position pos = xaie_pos_to_acap_pos(dev_handle.tile);

    /// The host and device must see the same kernel type so we need to build
    /// the kernel here.
    auto Kernel = kernel_builder(k);

    /// On device trigger outlining of device code.
    kernel_outliner<std::decay_t<decltype(Kernel)>>(Kernel);
#ifndef __SYCL_DEVICE_ONLY__
    /// Host side

    // The name is captured by its non-reference type and has to be in
    // the cl::sycl::detail namespace as the integration header is
    // defined to be in this namespace (and all our implementation
    // resides in trisycl by default, so ::detail resolves to
    // trisycl::detail)
    using KI = ::trisycl::detail::KernelInfo<std::decay_t<decltype(Kernel)>>;
    std::string kernelName = KI::getName();

    /// The sycl-chess script will build 2 versions per kernel: one with West
    /// parity and one with East parity
    if (hw::get_parity(pos) == hw::parity::west)
      kernelName += "_west";
    else
      kernelName += "_east";

    auto kernel_bin_data =
        ::trisycl::detail::program_manager::instance()->get_bin_data(kernelName);

    {
      // auto Transaction = dev_handle.get_transaction();
      dev_handle.core_reset();

      TRISYCL_DUMP2("Loading Kernel " << kernelName << " ELF to tile (" << pos.x
                                      << ',' << pos.y << ")",
                    "exec");

      dev_handle.load_elf_image(kernel_bin_data.Binary);

      TRISYCL_DUMP2("Loaded Kernel " << kernelName << " ELF to tile (" << pos.x
                                     << ',' << pos.y
                                     << ") beginning tile execution",
                    "exec");

      hw::offset_table ot;
      ot.global_variable_start = kernel_bin_data.MemSize;
      ot.global_variable_start =
          ot.global_variable_start & ~3; /// Align down to 4
      ot.heap_start =
          hw::offset_table::get_tile_mem_begin_offset() + mem_tile_size;
      ot.heap_start = acap::heap::align_size(ot.heap_start); // align up to 4;
      TRISYCL_DUMP2("tile(" << pos.x << ',' << pos.y << ") "
                            << "global_variable_start = 0x" << std::hex
                            << ot.global_variable_start << " heap_start = 0x"
                            << ot.heap_start,
                    "memory");
      dev_handle.memcpy_h2d(hw::offset_table::get_offset_table_begin_offset(),
                            &ot, sizeof(ot));
      acap::heap::init_allocator(dev_handle, ot.heap_start,
                                 /*heap_end*/ ot.global_variable_start -
                                     ot.heap_start);
      if (Phandler)
        /// if we have a Param handler let it run
        Phandler->template write_lambda<KI>(
            k,
            /// parameter should be placed inside the lambda of the kernel which
            /// is at the symbol kernel_lambda_capture
            kernel_bin_data.lookup_symbol("kernel_lambda_capture").addr &
                hw::offset_mask,
            ot.heap_start);

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
