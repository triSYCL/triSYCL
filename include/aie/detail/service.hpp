#ifndef AIE_DETAIL_SERVICE_HPP
#define AIE_DETAIL_SERVICE_HPP

/// This files contains default services that are always added to every kernel

#include "hardware.hpp"
#include "sync.hpp"
#include "xaie_wrapper.hpp"
#include <limits>
#include <iostream>

namespace aie::detail {

template <typename dev_handle>
struct send_log_service {
  /// This is an experimental improvement that adds support for float and double
  /// and move serialization from the device to the host.
  // struct str {
  //   /// Pointer to the first character of a buffer to print
  //   aie::generic_ptr<const char> data;
  //   /// Number of characters to print
  //   uint32_t size;
  // };
  // using data_type = std::variant<str, int64_t, double, float, const void*>;
  // template <typename Parent> struct add_to_service_api {
  //  private:
  //   template <typename T>
  //   __attribute__((noinline)) void log_internal(T str, bool chained)
  //     requires(std::is_same_v<T, const char*> || std::is_same_v<T, char*>)
  //   {
  //     send_log_service::str data { dev_ptr<const char>(str), strlen(str) };
  //     tile().perform_service(data_type { data }, chained);
  //   }
  //   template <typename T>
  //   __attribute__((noinline)) void log_internal(T data, bool chained)
  //     requires(std::is_integral_v<T>)
  //   {
  //     tile().perform_service(data_type { (int64_t)data }, chained);
  //   }
  //   template <typename T>
  //   __attribute__((noinline)) void log_internal(T data, bool chained)
  //     requires(std::is_same_v<T, double> || std::is_same_v<T, float>)
  //   {
  //     tile().perform_service(data_type { data }, chained);
  //   }
  //   template <typename T>
  //   __attribute__((noinline)) void log_internal(T data, bool chained)
  //     requires(std::is_same_v<T, const void*> || std::is_same_v<T, void*>)
  //   {
  //     tile().perform_service(data_type { (const void*)data }, chained);
  //   }

  /// This is the data that will be transmitted to the host when the device
  /// is logging. This struct needs to have the same layout on the host and the
  /// device.
  struct data_type {
    /// Pointer to the first character of a buffer to print
    aie::generic_ptr<const char> data;
    /// Number of characters to print
    uint32_t size;
  };
  template <typename Parent> struct add_to_service_api {
   private:
    void log_internal(const char* str, bool chained) {
      send_log_service::data_type data { generic_ptr<const char>(str),
                                         strlen(str) };
      tile().perform_service(data, chained);
    }
    void log_internal(int i, bool chained) {
      char arr[/*bits in base 2*/ 31 + /*sign*/ 1 + /*\0*/ 1];
      char* ptr = &arr[0];
      // host_breakpoint(ptr, i);
      write_number([&](char c) mutable { *(ptr++) = c; }, i);
      ptr[0] = '\0';
      log_internal(arr, chained);
    }
    auto& tile() { return *static_cast<Parent*>(this)->dt(); }

   public:
    /// Log inputs on the console as is.
    template <typename... Ts> __attribute__((noinline)) void log(Ts... ts) {
      int count = sizeof...(Ts) + 1;
      (log_internal(ts, /*chained all but last*/ --count), ...);
    }
    /// Log inputs on the console with a tile position indicator and end of line.
    template <typename... Ts> __attribute__((noinline)) void logln(Ts... ts) {
      log_internal("(", true);
      log_internal(tile().dyn_x(), true);
      log_internal(", ", true);
      log_internal(tile().dyn_y(), true);
      log_internal("): ", true);
      (log_internal(ts, true), ...);
      log_internal("\n", false);
    }
  };
  void act_on_data(int x, int y, dev_handle h, data_type dev_data) {
    // std::visit(
    //     [&](auto data) {
    //       if constexpr (std::is_same_v<decltype(data), str>) {
    //         std::string str;
    //         str.resize(data.size);
    //         /// Copy the indicated device data into a string.
    //         h.memcpy_d2h(str.data(), data.data, str.size());
    //         std::cout << str << std::flush;
    //       } else {
    //         std::cout << data << std::flush;
    //       }
    //     },
    //     dev_data);
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
    auto& tile() { return *static_cast<Parent*>(this)->dt(); }

   public:
    void abort() {
      exit_kernel(ec_abort);
    }
    void exit_kernel(int32_t ec = ec_success) {
      data_type data { ec };
      tile().perform_service(data);
    }
  };
  void act_on_data(int x, int y, dev_handle h, data_type d) {
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
