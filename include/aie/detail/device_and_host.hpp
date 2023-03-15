
#ifndef AIE_DETAIL_DEVICE_AND_HOST_HPP
#define AIE_DETAIL_DEVICE_AND_HOST_HPP

#if defined (__ACAP_EMULATION___)
#error "should only be used in host or device side for hardware"
#endif

#include "common.hpp"
#include "program_manager.hpp"
#include "xaie_wrapper.hpp"
#include "exec_kernel.hpp"

namespace aie::detail {

using lock_impl = lock_impl_fallback;

struct host_tile_impl : host_tile_impl_fallback {
xaie::handle dev_handle;
template <typename LambdaTy, typename DeviceTileTy>
void execute(LambdaTy l, DeviceTileTy& dt) {
  exec_kernel<DeviceTileTy> {}.exec(
      dev_handle, l, sizeof(typename DeviceTileTy::self_memory_tile),
      [&]<typename KI>(auto& L, uint32_t dev_addr, uint32_t heap_start) {
        write_lambda<KI>(L, dev_addr, heap_start);
      });
  }
#if !defined(__SYCL_DEVICE_ONLY__)
  template<typename DeviceImplTy>
  void init(DeviceImplTy &d, hw::position pos) {
    dev_handle = d.get_handle(pos);
  }
  ~host_tile_impl() {
    for (auto& elem : write_backs) {
      elem();
    }
  }
  std::vector<std::function<void()>> write_backs;
  /// Write the lambda on the device such that the kernel can use it.
  template <typename KernelDesc, typename KernelLambda>
  void write_lambda(KernelLambda& L, uint32_t dev_lambda_addr, uint32_t heap_start) {
    TRISYCL_DUMP2("Lambda address = " << (void*)(std::uintptr_t)dev_lambda_addr,
                  "memory");

    /// Write the lambda to memory, the accessors will get corrected later.
    dev_handle.store<KernelLambda, /*no_check*/ true>(dev_lambda_addr, L);
    for (int i = 0; i < KernelDesc::getNumParams(); i++) {
      kernel_param_desc_t kdesc = KernelDesc::getParamDesc(i);
      if (kdesc.kind != kernel_param_kind_t::kind_accessor)
        continue;
      auto* acc_addr = reinterpret_cast<host_accessor_impl*>(
          reinterpret_cast<char*>(&L) + kdesc.offset);
      device_accessor_impl dev_acc;
      dev_acc.size = acc_addr->size;
      dev_acc.elem_size = acc_addr->elem_size;
      unsigned size_in_bytes = dev_acc.size * dev_acc.elem_size;
      unsigned dev_data_addr =
          heap::malloc(dev_handle, heap_start, size_in_bytes);
      dev_handle.memcpy_h2d(dev_data_addr, acc_addr->data, size_in_bytes);
      dev_acc.data =
          hw::dev_ptr<char>::create(dev_handle.get_self_dir(), dev_data_addr);
      dev_handle.store<device_accessor_impl, /*no_check*/ true>(
          dev_lambda_addr + kdesc.offset, dev_acc);
      write_backs.push_back([=, dev_handle = dev_handle, host_addr = acc_addr->data]() mutable {
        dev_handle.memcpy_d2h(host_addr, dev_data_addr, size_in_bytes);
      });
    }
  }
#endif
  void register_accessor(const detail::accessor_common& accessor) {
  }
};

}

#endif
