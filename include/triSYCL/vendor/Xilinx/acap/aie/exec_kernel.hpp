
#ifdef __SYCL_DEVICE_ONLY__
#include "device_allocator.hpp"
#include "log.hpp"
#endif

#include <string>
#include "xaie_wrapper.hpp"

#include "triSYCL/detail/program_manager.hpp"

namespace trisycl::vendor::xilinx::acap::aie::detail {

template<typename TileHandle>
struct exec_kernel {
#ifdef __SYCL_DEVICE_ONLY__
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
static void kernel_prerun() {
  acap::heap::init_allocator();
}
static void kernel_postrun() {
  acap::heap::assert_no_leak();
  finish_kernel();
}
#else
static void kernel_prerun() {
}
static void kernel_postrun() {
}
#endif

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

  template<typename K>
  void exec(xaie::handle dev_handle, K k) {
     acap::hw::position pos = xaie_pos_to_acap_pos(dev_handle.tile);
    auto Kernel = kernel_builder(k);

#ifdef __SYCL_DEVICE_ONLY__

    // The outlining of the device binary Method 1: use it directly as a kernel
    // wrapper This still results in some "garbage" IR from the axi streams
    // default destructor, but you can run -O3 and it'll clean it up quite a bit
    // without nuking everything so it's progress. The result seems semi-
    // reasonable and passes through xchesscc at a reasonable speed
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

    if (hw::get_parity(pos) ==
        hw::parity::west)
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
      // auto Transaction = f.get_transaction();
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
}
