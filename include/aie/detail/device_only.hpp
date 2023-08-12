
#ifndef AIE_DETAIL_DEVICE_ONLY_HPP
#define AIE_DETAIL_DEVICE_ONLY_HPP

/// This file contains parts of the hardware execution that are only used on
/// device

#if defined(__AIE_EMULATION___) || !defined(__SYCL_DEVICE_ONLY__)
#error "should only be used on the host side of hardware execution"
#endif

#include "aie-intrinsic.h"
#include "common.hpp"
#include "device_and_host.hpp"

namespace aie::detail {

/// The type name must match between the host and device because the kernel name
/// depends on it. So we use a struct here to create a new type with the correct
/// name instead of a using that would generate an alias with the same mangling
/// as the original type.
struct device_mem_handle_impl : device_mem_handle_impl_fallback {};

using device_mem_handle = device_mem_handle_adaptor<device_mem_handle_impl>;

struct device_lock_impl {
 private:
  int id;

 public:
  device_lock_impl(dir d, int i)
      : id { (detail::underlying_value(d) * 16) + i } {
    assert(i < 16);
    assert(id < 64);
  }
  void acquire() { aie::intrinsics::acquire(id); }
  void release() { aie::intrinsics::release(id); }
  void acquire_with_value(bool val) { aie::intrinsics::acquire(id, val); }
  void release_with_value(bool val) { aie::intrinsics::release(id, val); }
};

using host_lock_impl = lock_impl_fallback;
using device_impl = device_impl_fallback;
using device_mem_handle = device_mem_handle_adaptor<device_mem_handle_impl>;

struct device_tile_impl : device_tile_impl_fallback {
  template <dir d> void* get_mem_addr() {
    return get_object<void>(offset_table::get_tile_mem_begin_offset(),
                                d);
  }
  void stream_write16(const char* ptr, int stream_dix) {
    aie::intrinsics::stream_write16(ptr, stream_dix);
  }
  void stream_read16(char* ptr, int stream_dix) {
    aie::intrinsics::stream_read16(ptr, stream_dix);
  }
  void cascade_write48(const char* ptr) { aie::intrinsics::cstream_write48(ptr); }
  void cascade_read48(char* ptr) { aie::intrinsics::cstream_read48(ptr); }
  int x_coord() { return get_tile_x_coordinate(); }
  int y_coord() { return get_tile_y_coordinate(); }
  device_lock_impl lock(position pos, dir d, int i) {
    return device_lock_impl(resolved_self_dir(pos, d), i);
  }
  template <typename T, typename ServiceTy>
  static service_info<T>::ret_t
  perform_service(service_info<T>::data_t d, bool chained = false) {
    volatile service_device_side* obj = service_device_side::get();
    typename service_info<T>::non_void_ret_t ret;
    typename service_info<T>::data_t data;
    volatile_store(&data, d);

    obj->index = ServiceTy::template get_index<T>;
    obj->data = &data;
    obj->ret = &ret;
    obj->chained_request = chained;

    /// Notify the host of the data being available.
    obj->barrier.wait();

    /// Wait for the host to process the data.
    obj->barrier.wait();

    if constexpr (service_info<T>::is_void_ret) {
      return;
    } else {
      return volatile_load(&ret);
    }
  }
};

/// a type with a similar API to the real device_tile.
/// This type only needs to implement the API that is used by the basic
/// services.
struct fake_dt {
  using BasicService = service_list_info<done_service<device_mem_handle>,
                                         send_log_service<device_mem_handle>>;
  template <typename T> auto perform_service(T data, bool chained = false) {
    T local = data;
    /// lookup what type of service sends this data;
    using service_t = BasicService::template get_info_t<
        BasicService::data_seq::template get_index<std::decay_t<T>>>::service_t;
    /// Call on to the implementation to execute the service correct
    return device_tile_impl::perform_service<service_t, BasicService>(local,
                                                                       chained);
  }
  int dyn_x() { return get_tile_x_coordinate(); }
  int dyn_y() { return get_tile_y_coordinate(); }
};

/// Function to obtain a mix-in of all basic service APIs
detail::multi_service_accessor<fake_dt, done_service<device_mem_handle>,
                               send_log_service<device_mem_handle>>
basic_service() {
  fake_dt dt;
  return { &dt };
}

} // namespace aie::detail

/// Notify the host that the kernel has finished.
extern __attribute__((noreturn)) void finish_kernel(int32_t exit_code) {
  aie::detail::basic_service().exit_kernel(exit_code);
  while (1)
    aie::intrinsics::memory_fence();
}

/// The assert macro will call this function if an assertion fails on device.
extern __attribute__((noreturn)) void __assert_fail(const char* expr,
                                                    const char* file,
                                                    unsigned int line,
                                                    const char* func) {
  aie::detail::basic_service().logln("aie(", aie::get_tile_x_coordinate(),
                                     ", ", aie::get_tile_y_coordinate(),
                                     ") at ", file, ":", line, ": ", func,
                                     ": Assertion `", expr, "' failed\n\n");
  finish_kernel(aie::detail::ec_assert);
}

#endif
