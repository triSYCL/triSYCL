
#ifndef AIE_DETAIL_DEVICE_ONLY_HPP
#define AIE_DETAIL_DEVICE_ONLY_HPP

#if defined(__AIE_EMULATION___) || !defined(__SYCL_DEVICE_ONLY__)
#error "should only be used on the host side of hardware execution"
#endif

#include "acap-intrinsic.h"
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
  device_lock_impl(hw::dir d, int i)
      : id { (detail::underlying_value(hw::resolved_dir(d)) * 16) + i } {
    assert(i < 16);
    assert(id < 64);
  }
  void acquire() { acap_intr::acquire(id); }
  void release() { acap_intr::release(id); }
  void acquire_with_value(bool val) { acap_intr::acquire(id, val); }
  void release_with_value(bool val) { acap_intr::release(id, val); }
};

using host_lock_impl = lock_impl_fallback;
using device_impl = device_impl_fallback;
using device_mem_handle = device_mem_handle_adaptor<device_mem_handle_impl>;

struct device_tile_impl : device_tile_impl_fallback {
  template <hw::dir d> void* get_mem_addr() {
    return hw::get_object<void>(hw::offset_table::get_tile_mem_begin_offset(),
                                d);
  }
  void stream_write16(const char* ptr, int stream_dix) {
    acap_intr::stream_write16(ptr, stream_dix);
  }
  void stream_read16(char* ptr, int stream_dix) {
    acap_intr::stream_read16(ptr, stream_dix);
  }
  void cascade_write48(const char* ptr) { acap_intr::cstream_write48(ptr); }
  void cascade_read48(char* ptr) { acap_intr::cstream_read48(ptr); }
  int x_coord() { return hw::get_tile_x_coordinate(); }
  int y_coord() { return hw::get_tile_y_coordinate(); }
  device_lock_impl lock(hw::dir d, int i) { return device_lock_impl(d, i); }
  template <typename T, typename ServiceTy>
  __attribute__((noinline)) static service_info<T>::ret_t
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

/// This is used to provide access to the default services for __assert_fail and
/// finish_kernel. Since the done_service and send_log_service are always added at the
/// beginning in aie.hpp, their indexes are always 0 and 1 respectively.
template<typename T>
service_info<T>::ret_t basic_service(typename service_info<T>::data_t data, bool chained = false) {
  using BasicService = service_list_info<done_service<device_mem_handle>, send_log_service<device_mem_handle>>;
  return device_tile_impl::perform_service<T, BasicService>(data, chained);
}

 __attribute__((noinline)) void log_internal(std::string_view sv, bool chained) {
  send_log_service<device_mem_handle>::data_type data{hw::dev_ptr<const char>(sv.data()), sv.size()};
  basic_service<send_log_service<device_mem_handle>>(data, chained);
}

 __attribute__((noinline)) void log_internal(int i, bool chained) {
  char arr[/*bits in base 2*/ 31 + /*sign*/ 1 + /*\0*/ 1];
  char* ptr = arr;
  write_number([&](char c) { *ptr++ = c; }, i);
  *ptr = '\0';
  log_internal(std::string_view(arr, ptr - arr), chained);
}

template<typename Type, typename ...Types>
void debug_log(Type First, Types... Others) {
  /// The first will have coordinates
  log_internal(First, /*chained*/sizeof...(Types));
  int count = sizeof...(Types) + 1;
  /// The others, if any,  will not have coordinates
  (log_internal(Others, /*chained all but last*/--count), ...);
}

} // namespace aie::detail

/// Notify the host that the kernel has finished.
extern __attribute__((noreturn)) __attribute__((noinline)) void
finish_kernel(int32_t exit_code) {
  // aie::detail::debug_log("start finish\n");
  aie::detail::basic_service<aie::detail::done_service<aie::detail::device_mem_handle>>(
      aie::detail::done_service<aie::detail::device_mem_handle>::data_type {exit_code});
  // aie::detail::debug_log("done\n");
  while (1)
    acap_intr::memory_fence();
}

/// The assert macro will call this function if an assertion fails on device.
extern __attribute__((noreturn)) __attribute__((noinline)) void
__assert_fail(const char* expr, const char* file, unsigned int line,
              const char* func) {
  aie::detail::debug_log("aie(", aie::hw::get_tile_x_coordinate(), ", ",
                         aie::hw::get_tile_y_coordinate(), ") at ", file, ":",
                         line, ": ", func, ": Assertion `", expr, "' failed\n\n");
  finish_kernel(aie::detail::ec_assert);
}

#endif
