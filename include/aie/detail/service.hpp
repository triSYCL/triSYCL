#ifndef AIE_DETAIL_SERVICE_HPP
#define AIE_DETAIL_SERVICE_HPP

#include "hardware.hpp"
#include "sync.hpp"
#include "xaie_wrapper.hpp"
#include <limits>
#include <iostream>

namespace aie::detail {

template <typename dev_handle>
struct send_log_service {
  /// This is the data that will be transmitted to the host when the device
  /// is logging. This struct needs to have the same layout on the host and the
  /// device.
  struct data_type {
    /// Pointer to the first character of a buffer to print
    generic_ptr<const char> data;
    /// Number of characters to print
    uint32_t size;
  };
  template <typename Parent> struct add_to_service_api {
   private:
    __attribute__((noinline)) void log_internal(const char* str, bool chained) {
      send_log_service::data_type data { hw::dev_ptr<const char>(str),
                                     strlen(str) };
      tile().perform_service(data);
    }
    __attribute__((noinline)) void log_internal(int i, bool chained) {
      char arr[/*bits in base 2*/ 31 + /*sign*/ 1 + /*\0*/ 1];
      char* ptr = &arr[0];
      // host_breakpoint(ptr, i);
      write_number([&](char c) mutable { *(ptr++) = c; }, i);
      ptr[0] = '\0';
      log_internal(arr, chained);
    }
    auto& tile() { return *static_cast<Parent*>(this)->dt(); }

   public:
    template <typename... Ts>
    void log(Ts... ts) {
      int count = sizeof...(Ts) + 1;
      (log_internal(ts, /*chained all but last*/ --count), ...);
    }
  };
  static void act_on_data(int x, int y, dev_handle h, data_type dev_data) {
    std::string str;
    str.resize(dev_data.size);
    /// Copy the indicated device data into a string.
    h.memcpy_d2h(str.data(), dev_data.data, str.size());
    std::cout << str << std::flush;
    return;
  }
};

enum : int32_t {
  ec_success,
  ec_abort = std::numeric_limits<int32_t>::max() - 1,
  ec_assert = std::numeric_limits<int32_t>::max(),
};

template <typename dev_handle>
struct done_service {
  /// it needs no data, since it is just a signal.
  struct data_type {
    int32_t ec;
  };
  template <typename Parent> struct add_to_service_api {
   private:
    auto* tile() { return static_cast<Parent*>(this)->dt(); }

   public:
    void abort() {
      exit_kernel(ec_abort);
    }
    void exit_kernel(int32_t ec = ec_success) {
      data_type data { ec };
      tile().perform_service(data);
    }
  };
  static void act_on_data(int x, int y, dev_handle h, data_type d) {
    if (d.ec == ec_assert) {
      assert(false && "propagating assert form the device to the host");
    } else if (d.ec == ec_abort) {
      std::abort();
    } else if (d.ec != ec_success) {
      std::exit(d.ec);
    }
  }
};

} // namespace aie::detail

#endif
