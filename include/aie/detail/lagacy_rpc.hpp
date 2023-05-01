#ifndef AIE_DETAIL_LEGACY_RPC_HPP
#define AIE_DETAIL_LEGACY_RPC_HPP

#include <cstdint>
#include <functional>
#include <variant>
#include "hardware.hpp"
#ifndef __SYCL_DEVICE_ONLY__
#include <iostream>
#endif

namespace aie::detail {

struct soft_barrier {
#if defined(__SYCL_XILINX_AIE__)
  enum {
    device = 0,
    host = 1,
  };

#if !defined(__SYCL_DEVICE_ONLY__)
  struct host_side {
    xaie::handle h;
    uint32_t dev_offset;
    /// Wait for the device to reach the barrier.
    void wait() {
      trisycl::detail::no_log_in_this_scope nls;
      uint32_t counter =
          h.mem_read(dev_offset + offsetof(device_side, counters[host]));
      /// Wait for the device to arrive.
      while (h.mem_read(dev_offset + offsetof(device_side, counters[device])) ==
             counter)
        std::this_thread::yield();
      // Notify the device that the host has arrived too
      counter++;
      h.mem_write(dev_offset + offsetof(device_side, counters[host]), counter);
    }
    /// Return true if the device was waiting and perform the barrier.
    /// Return false if the device was not waiting and doesn't perform the
    /// barrier.
    bool try_arrive() {
      trisycl::detail::no_log_in_this_scope nls;
      uint32_t counter =
          h.mem_read(dev_offset + offsetof(device_side, counters[host]));
      if (h.mem_read(dev_offset + offsetof(device_side, counters[device])) ==
          counter)
        // If the device has not arrived, the host does not arrive either
        return false;
      // Notify the device that the host has arrived too
      counter++;
      h.mem_write(dev_offset + offsetof(device_side, counters[host]), counter);
      return true;
    }
  };
#endif
  class device_side {
#if !defined(__SYCL_DEVICE_ONLY__)
    /// Allow host_side to use offsetof on members
    friend struct soft_barrier::host_side;
#endif
    /// This is volatile because it is used by the host and device concurrently
    /// so the value may spuriously change and cannot be predicted by the
    /// compiler. counters[device] will be written to by the device and read by
    /// the host and counters[host] will be written to by the host and read by
    /// the device.
    volatile uint32_t counters[2];

  public:
#if defined(__SYCL_DEVICE_ONLY__)
    /// The device_side cannot be moved on device because the host_side depends
    /// on its address. The device_side should be initialized to 0 from the host
    /// before the kernel starts.
    device_side() = delete;
    device_side(const device_side &) = delete;
#endif
    /// Wait for the host.
    void wait() volatile {
      /// Prevent memory operation from being moved below the barrier
      acap_intr::memory_fence();
      /// Notify the host that the device has arrived
      counters[device] = counters[device] + 1;
      /// Wait for the host to arrive.
      while (counters[host] != counters[device]) {
        /// counters[host] and counters[device] are volatile so this loop is not
        /// dead.
      }
      /// Prevent memory operation from being moved above the barrier
      acap_intr::memory_fence();
    }
  };
#endif
};

/// This is a convenience wrapper that allows creating functor based RPC easily.
template<typename DataTy>
struct functor_rpc {
  using data_type = DataTy;

#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  std::function<uint32_t(int, int, xaie::handle, data_type)> impl{};

  /// This is only one instance of this per template parameter, this function
  /// will get that instance.
  static functor_rpc &get() {
    static functor_rpc val;
    return val;
  }

  template<typename Func>
  static void set_handler(Func&& func) {
    get().impl = std::forward<Func>(func);
  }

  /// Process data that is received.
  /// This is executed each time new data arrives.
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    return get().impl(x, y, h, d);
  }
#endif
};

/// This is the data that will be transmitted from the device to the host to
/// update the image. This struct needs to have the same layout on the host and
/// the device.
struct image_update_data {
  /// A pointer to the new image data
  hw::dev_ptr<void> data;

  /// In min_value and max_value, uint64_t is just to have 8-byte of storage,
  /// the graphics system will bitcast min_value to the proper type before use.

  /// 8-byte storage for the minimun value of a pixel.
  uint64_t min_value;
  /// 8-byte storage for The maximum value of a pixel.
  uint64_t max_value;

  /// Number of image updates that as been sent
  uint32_t counter;
};

/// The act_on_data has dependencies on graphics.hpp but we do not want to
/// include graphics.hpp here because:
///  - rpc.hpp is already included in graphics.hpp and we cannot have circular
///    dependencies. this might be removable with sufficient refactoring.
///  - rpc.hpp is a lower level system then graphics.hpp so
///    graphics.hpp should include rpc.hpp not the inverse.
/// so we use functor_rpc to have a type-erased RPC
/// class. and the functor is setup in graphics.hpp.
using image_update_rpc = functor_rpc<image_update_data>;

struct send_log_rpc {
  /// This is the data that will be transmitted to the host when the device
  /// is logging. This struct needs to have the same layout on the host and the
  /// device.
  struct data_type {
    /// Pointer to the first character of a buffer to print
    hw::dev_ptr<const char> data;
    /// Number of characters to print
    uint32_t size;
  };
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  static uint32_t act_on_data(int x, int y, xaie::handle h,
                              data_type dev_data) {
    std::string str;
    str.resize(dev_data.size);
    /// Copy the indicated device data into a string.
    h.moved(hw::position{x, y}.moved(dev_data.data.get_dir()))
        .memcpy_d2h(str.data(), dev_data.data.get_offset(), str.size());
    std::cout << str << std::flush;
    return 0;
  }
#endif
};

struct host_breakpoint {
  struct data_type {
    uint32_t offset;
    uint32_t count;
  };
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  __attribute__((noinline)) static void host_break(int x, int y, xaie::handle h, data_type d) {
    /// Put a breakpoint in this function to debug
  }
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    host_break(x, y, h, d);
    return 0;
  }
#endif
};

/// done_rpc is handled differently because indicate if a kernel is done
/// executing.
struct done_rpc {
  /// it needs no data, since it is just a signal.
  struct data_type {};
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  static uint32_t act_on_data(int x, int y, xaie::handle h, data_type d) {
    /// The effect of the signal are handled directly by wait_all
    assert(false && "This should be handled by wait_all");
    return 0;
  }
#endif
};

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename... Tys> struct rpc_impl {
  using Var = std::variant<typename Tys::data_type...>;
  // using RetVar = std::variant<rpc_ret_ty<Tys>...>;
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  struct host_side {
    int x_size, y_size;
    xaie::handle h;

    /// this will retrun a handle to the synchronization barrier between the device and the host.
    soft_barrier::host_side get_barrier(int x, int y) {
      return {h.moved(x, y), (uint32_t)(hw::offset_table::get_rpc_record_begin_offset() + offsetof(device_side, barrier))};
    }

    /// This will invoke the correct function to process the data in v
    uint32_t visit(int x, int y, xaie::handle h, Var v) {
      auto visitor = detail::overloaded{[&](typename Tys::data_type data) {
        return Tys::act_on_data(x, y, h, data);
      }...};
      return std::visit(visitor, v);
    }

    /// This will wait on every kernel while handling there RPC requests
    void wait_all() {
      trisycl::detail::no_log_in_this_scope nls;
      int addr = hw::offset_table::get_rpc_record_begin_offset();
      /// This count the number of kernel that indicated they finished
      /// executing. any kernel can signal it finished executing just once
      /// because it stop executing or get stuck in an infinite loop after that.
      /// so it is not needed to keep track of which kernel stoped executing
      /// just how many.
      int done_counter = 0;
      do {
        for (int x = 0; x < x_size; x++)
          for (int y = 0; y < y_size; y++) {
            /// We process at least one request per device.
            bool chain = true;
            /// while the device asks to chain requests.
            while (chain) {
              /// If try_arrive returns true the device has written data and is
              /// waitng on the host to act on it
              if (!get_barrier(x, y).try_arrive())
                continue;
              device_side ds = h.moved(x, y).load<device_side>(addr);
              Var data = h.moved(x, y).load(ds.data);
              /// Read the data the device has written.
              /// Deal with the special case of a kernel indicating it is done.
              /// This kernel stopped executing.
              if (data.index() == 0) {
                done_counter++;
              } else {
                /// Otherwise call the appropriate function.
                auto ret = visit(x, y, h.moved(x, y), data);
                /// And write back the response.
                h.moved(x, y).mem_write(addr + offsetof(device_side, ret_val),
                                        ret);
                /// read if the device requested to chain the is request.
                chain = ds.chained_request;
              }
              get_barrier(x, y).wait();
            }
          }
        /// Wait until all kernels have finished.
      } while (done_counter < x_size * y_size);
    }
  };
#endif

  struct device_side {
#if defined(__SYCL_XILINX_AIE__)
    static device_side *get() {
      return hw::get_object<device_side>(
          hw::offset_table::get_rpc_record_begin_offset());
    }
    soft_barrier::device_side barrier;
    hw::dev_ptr<Var> data;
    uint32_t ret_val;

    /// This asks the host to wait for on other request from the same device
    /// after processing this request. this exist to prevent le host from
    /// interleaving log_internal requests.
    uint32_t chained_request;

    /// This sent data to the host to be processed.
    template <typename Ty> uint32_t perform(Ty &&d, bool chained = false) {
      /// Write the data.
      Var v = d;
      data = &v;
      chained_request = chained;
      /// Notify the host of the data being available.
      barrier.wait();
      /// Wait for the host to process the data.
      barrier.wait();
      return ret_val;
    }
#endif
  };
};

// using rpc = rpc_impl<done_rpc, image_update_rpc, send_log_rpc, host_breakpoint>;

#if defined(__SYCL_XILINX_AIE__)
namespace {
detail::assert_equal_layout<
    rpc::device_side, rpc::device_side,
    /*expected_size=*/hw::offset_table::get_rpc_record_size()>
    rpc_check;
}
#endif

#ifdef __SYCL_DEVICE_ONLY__

/// When this function is called the host will call a function names host_break.
/// and we can put breakpoints on the hsot and study the state of the device.
 template <typename... Ts> __attribute__((noinline)) void host_breakpoint(Ts... t) {
  host_breakpoint::data_type dt{};
  /// Inform the host via RPC that the kernel has finished
  uint32_t arr[] = {hw::bit_cast<uint32_t>(t)..., 0};
  dt.offset = (sizeof...(Ts)) ? ((std::uintptr_t)&arr & hw::offset_mask) : 0;
  dt.count = sizeof...(Ts);

  rpc::device_side::get()->perform(dt);
}

  /// Log the tile coordinates followed by the null-terminated string provided as argument
  __attribute__((noinline)) void log_internal(const char *ptr, bool with_coord = true, bool chained = false) {
    /// Send a null-terminated string through the RPC system to the host.
    auto send_str = [](const char *ptr, bool chained) {
      send_log_rpc::data_type sld;
      sld.data = ptr;
      sld.size = hw::strlen(ptr);
      rpc::device_side::get()->perform(sld, chained);
    };
    if (with_coord) {
      char arr[] = "00, 0 : ";
      arr[0] = '0' + (hw::get_tile_x_coordinate() / 10);
      arr[1] = '0' + (hw::get_tile_x_coordinate() % 10);
      arr[4] = '0' + (hw::get_tile_y_coordinate() % 10);

      send_str(arr, true);
    }
    send_str(ptr, chained);
  }

  /// This function serializes a number into a buffer.
  /// \param[in] write is a callable handling each generated character, for example to add a character to a buffer,
  /// \param[in] i is the number to be represented as characters
  /// \param[in] base_char is the sequence of characters to use as a base. A
  /// mapping between a digit and the character representation of such digit.
  static void write_number(auto write, int i,
                           const char *base_char = "0123456789") {
    if (i < 0)
      write('-');
    /// For 0 print 0 instead of nothing.
    if (i == 0) {
      write(base_char[0]);
      return;
    }

    int base = strlen(base_char);
    int digit_count = 1;
    int tmp = i;
    while (tmp >= base || tmp <= -base) {
      digit_count++;
      tmp = tmp / base;
    }
    for (int d = digit_count; d > 0; d--)
      write(base_char[(hw::abs(i / hw::pow(base, d - 1)) % base)]);
  }

  /// This will log_internal i on the console of the host.
  /// This __attribute__((noinline)) is just to make the IR more readable. It is
  /// not required for any other reason.
  __attribute__((noinline)) void log_internal(int i, bool with_coord = true, bool chained = false, const char* base = "0123456789") {
    char arr[/*bits in base 2*/31 + /*sign*/1 + /*\0*/1];
    char *ptr = &arr[0];
    // host_breakpoint(ptr, i);
    write_number([&](char c) mutable { *(ptr++) = c; }, i, base);
    ptr[0] = '\0';
    log_internal(arr, with_coord, chained);
  }

  /// This __attribute__((noinline)) is just to make the IR more readable. it is
  /// not required for any other reason.
  /// This will log_internal a pointer in hexadecimal preceded by 0x on the console of the host
  __attribute__((noinline)) void log_internal(void* p, bool with_coord = true, bool chained = false) {
    log_internal("0x", with_coord, true);
    log_internal(reinterpret_cast<std::intptr_t>(p), false, chained, "0123456789abcdef");
  }
#else
  inline void log_internal(const char* ptr, bool = false, bool = false) {
    std::cout << ptr;
  }
  inline void log_internal(void* ptr, bool = false, bool = false) {
    std::cout << ptr;
  }
  inline void log_internal(int i, bool = false, bool = false) {
    std::cout << i;
  }
#endif

template<typename Type, typename ...Types>
void log(Type First, Types... Others) {
  int count = sizeof...(Types);
  /// The first will have coordinates
  log_internal(First, /*with_coord*/true, /*chained*/count);
  /// The others, if any,  will not have coordinates
  (log_internal(Others, /*with_coord*/false, /*chained all but last*/--count), ...);
}

}

#if defined(__SYCL_DEVICE_ONLY__)
/// Notify the host that the kernel has finished.
__attribute__((noreturn)) void finish_kernel() {
  aie::detail::done_rpc::data_type dt{};
  /// Inform the host via RPC that the kernel has finished
  aie::detail::rpc::device_side::get()->perform(dt);
  aie::detail::log("exiting\n");
  while (1)
    acap_intr::memory_fence();
}


/// The assert macro will call this function if an assertion fails on device.
void __assert_fail(const char *expr, const char *file, unsigned int line,
                   const char *func) {
  aie::detail::log("acap : ", file, ":", line, ": ",
                                           func, ": Assertion `", expr,
                                           "' failed\n", "kernel killed\n");
  finish_kernel();
}

#endif

#endif
