#ifndef AIE_DETAIL_RPC_HPP
#define AIE_DETAIL_RPC_HPP

#include "hardware.hpp"
#include "sync.hpp"
#include "xaie_wrapper.hpp"
#include <limits>

namespace aie::detail {

template <typename dev_handle>
struct send_log_rpc {
  /// This is the data that will be transmitted to the host when the device
  /// is logging. This struct needs to have the same layout on the host and the
  /// device.
  struct data_type {
    /// Pointer to the first character of a buffer to print
    generic_ptr<const char> data;
    /// Number of characters to print
    uint32_t size;
  };
  template <typename Parent> struct add_to_dev_handle {
   private:
    __attribute__((noinline)) void log_internal(const char* str, bool chained) {
      send_log_rpc::data_type data { hw::dev_ptr<const char>(str),
                                     strlen(str) };
      get()->perform_rpc(data);
    }
    __attribute__((noinline)) void log_internal(int i, bool chained) {
      char arr[/*bits in base 2*/ 31 + /*sign*/ 1 + /*\0*/ 1];
      char* ptr = &arr[0];
      // host_breakpoint(ptr, i);
      write_number([&](char c) mutable { *(ptr++) = c; }, i);
      ptr[0] = '\0';
      log_internal(arr, chained);
    }
    auto* get() { return static_cast<Parent*>(this)->dt(); }

   public:
    template <typename First, typename... Ts>
    void log(First first, Ts... others) {
      /// The first will have coordinates
      log_internal(first, /*chained*/ sizeof...(Ts));
      int count = sizeof...(Ts) + 1;
      /// The others, if any,  will not have coordinates
      (log_internal(others, /*chained all but last*/ --count), ...);
    }
  };
  static void act_on_data(int x, int y, dev_handle h, data_type dev_data) {
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
    std::string str;
    str.resize(dev_data.size);
    /// Copy the indicated device data into a string.
    // h.moved(hw::position { x, y }.moved(dev_data.data.get_dir()))
    //     .memcpy_d2h(str.data(), dev_data.data.get_offset(), str.size());
    h.memcpy_d2h(str.data(), dev_data.data, str.size());
    std::cout << str << std::flush;
#endif
    return;
  }
};

enum : int32_t {
  ec_success,
  ec_abort = std::numeric_limits<int32_t>::max() - 1,
  ec_assert = std::numeric_limits<int32_t>::max(),
};

template <typename dev_handle>
struct done_rpc {
  /// it needs no data, since it is just a signal.
  struct data_type {
    int32_t ec;
  };
  template <typename Parent> struct add_to_dev_handle {
   private:
    auto* get() { return static_cast<Parent*>(this)->dt(); }

   public:
    void abort() {
      exit_kernel(ec_abort);
    }
    void exit_kernel(int ec = ec_success) {
      data_type data { ec };
      get()->perform_rpc(data);
    }
  };
  static uint32_t act_on_data(int x, int y, dev_handle h, data_type d) {
    if (d.ec == ec_assert) {
      assert(false && "propagating assert form the device to the host");
    } else if (d.ec == ec_abort) {
      std::abort();
    } else if (d.ec != ec_success) {
      std::exit(d.ec);
    }
    return 0;
  }
};

} // namespace aie::detail

#endif
