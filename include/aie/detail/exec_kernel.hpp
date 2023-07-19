#ifndef AIE_DETAIL_EXEC_KERNEL_HPP
#define AIE_DETAIL_EXEC_KERNEL_HPP

#include "kernel_desc.hpp"
#ifndef __SYCL_XILINX_AIE__
#error "This header should only be included when targeting hardware"
#endif

/** \file

    This file implements the kernel outlining and execution logic when executing
    on hardware

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

// #include "device_allocator.hpp"
#include "common.hpp"
#include "hardware.hpp"
// #include "log.hpp"
#include "device_allocator.hpp"
#include "program_manager.hpp"
#include "sync.hpp"
#include "xaie_wrapper.hpp"

#include <string>

extern void finish_kernel(int32_t exit_code);

#ifdef __SYCL_DEVICE_ONLY__

/// This declaration will invoke the constructors of every global variable in
/// the file.
extern "C" void __cxx_global_var_ctor() {}

/// This will be filled by our compiler with every destructor.
extern "C" void __cxx_global_var_dtor() {}

#endif

namespace aie::detail {

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
  static void kernel_prerun() { __cxx_global_var_ctor(); }
  /// This will be invoked on device when exiting normally after any user code.
  static void kernel_postrun() {
    __cxx_global_var_dtor();
    // heap::assert_no_leak();
    finish_kernel(0);
  }
  /// Generic kernel invoker on device.
  template <typename KernelType, typename Func>
  static void kernel_run(Func func) {
    kernel_prerun();
    func(*get_object<KernelType>(
        offset_table::get_lambda_begin_offset()));
    kernel_postrun();
  }

#else
  template <typename KernelType, typename Func>
  static void kernel_run(Func func) {}
#endif

  template <typename KernelType>
  auto kernel_builder(KernelType& k)
    requires requires(KernelType k, TileHandle t) { k(t); }
  {
    return [] {
      kernel_run<KernelType>([](auto& k) {
        TileHandle t {};
        k(t);
      });
    };
  }

  template <typename K, typename LambdaTy>
  void exec(xaie::handle dev_handle, K k, uint32_t mem_tile_size,
            LambdaTy store_lambda) {
    /// The host and device must see the same kernel type so we need to build
    /// the kernel here.
    auto Kernel = kernel_builder(k);

    /// On device trigger outlining of device code.
    kernel_outliner<std::decay_t<decltype(Kernel)>>(Kernel);
#ifndef __SYCL_DEVICE_ONLY__
    /// Host side

    position pos = xaie_pos_to_aie_pos(dev_handle.tile);

    // The name is captured by its non-reference type and has to be in
    // the sycl::detail namespace as the integration header is
    // defined to be in this namespace (and all our implementation
    // resides in trisycl by default, so ::detail resolves to
    // trisycl::detail)
    using KI = aie::detail::KernelInfo<std::decay_t<decltype(Kernel)>>;
    std::string kernelName = KI::getName();

    /// The sycl-chess script will build 2 versions per kernel: one with West
    /// parity and one with East parity
    if (get_parity(pos) == parity::west)
      kernelName += "_west";
    else
      kernelName += "_east";

    auto kernel_bin_data =
        program_manager::instance()->get_bin_data(kernelName);

    {
      TRISYCL_DUMP2("Loading Kernel " << kernelName << " ELF to tile (" << pos.x
                                      << ',' << pos.y << ")",
                    "exec");

      dev_handle.load_elf_image(kernel_bin_data.Binary);

      TRISYCL_DUMP2("Loaded Kernel " << kernelName << " ELF to tile (" << pos.x
                                     << ',' << pos.y
                                     << ") beginning tile execution",
                    "exec");

      offset_table ot(kernel_bin_data.MemSize, mem_tile_size, sizeof(K));
      TRISYCL_DUMP2("tile(" << pos.x << ',' << pos.y << ") "
                            << "global_variable_start = 0x" << std::hex
                            << ot.global_variable_start << " heap_start = 0x"
                            << ot.heap_start,
                    "memory");
      dev_handle.store(offset_table::get_offset_table_begin_offset(), ot);
      heap::init_allocator(dev_handle, ot.heap_start,
                           /*heap_size*/ ot.global_variable_start -
                               ot.heap_start);

      store_lambda.template operator()<KI>(k, ot.lambda_start, ot.heap_start);

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
} // namespace aie::detail

#endif
