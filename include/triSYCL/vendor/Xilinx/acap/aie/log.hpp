#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LOG_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LOG_HPP

/** \file

    Contains logging fontionalities

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "hardware.hpp"
#include "rpc.hpp"

namespace trisycl::vendor::xilinx::acap {

#ifdef __SYCL_DEVICE_ONLY__

/// When this function is called the host will call a function names host_break.
/// and we can put breakpoints on the hsot and study the state of the device.
 template <typename... Ts> __attribute__((noinline)) void host_breakpoint(Ts... t) {
  acap::aie::host_breakpoint::data_type dt{};
  /// Inform the host via RPC that the kernel has finished
  uint32_t arr[] = {hw::bit_cast<uint32_t>(t)..., 0};
  dt.offset = (sizeof...(Ts)) ? ((std::uintptr_t)&arr & hw::offset_mask) : 0;
  dt.count = sizeof...(Ts);

  acap::aie::rpc::device_side::get()->perform(dt);
}

/// Send a null-terminated string through the RPC system to the host.
__attribute__((noinline)) void log_internal(const char *ptr, bool chained) {
  aie::send_log_rpc::data_type sld;
  sld.data = ptr;
  sld.size = hw::strlen(ptr);
  acap::aie::rpc::device_side::get()->perform(sld, chained);
}

  /// Log the tile coordinates followed by the null-terminated string provided as argument
  __attribute__((noinline)) void log(const char *ptr, bool with_coord = true, bool chained = false) {
    if (with_coord) {
      char arr[] = "00, 0 : ";
      arr[0] = '0' + (hw::get_tile_x_coordinate() / 10);
      arr[1] = '0' + (hw::get_tile_x_coordinate() % 10);
      arr[4] = '0' + (hw::get_tile_y_coordinate() % 10);

      log_internal(arr, true);
    }
    log_internal(ptr, chained);
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

  /// This will log i on the console of the host.
  /// This __attribute__((noinline)) is just to make the IR more readable. It is
  /// not required for any other reason.
  __attribute__((noinline)) void log(int i, bool with_coord = true, bool chained = false, const char* base = "0123456789") {
    char arr[/*bits in base 2*/31 + /*sign*/1 + /*\0*/1];
    char *ptr = &arr[0];
    // host_breakpoint(ptr, i);
    write_number([&](char c) mutable { *(ptr++) = c; }, i, base);
    ptr[0] = '\0';
    log(arr, with_coord, chained);
  }

  /// This __attribute__((noinline)) is just to make the IR more readable. it is
  /// not required for any other reason.
  /// This will log a pointer in hexadecimal preceded by 0x on the console of the host
  __attribute__((noinline)) void log(void* p, bool with_coord = true, bool chained = false) {
    log("0x", with_coord, true);
    log(reinterpret_cast<std::intptr_t>(p), false, chained, "0123456789abcdef");
  }
#else
  inline void log(const char* ptr, bool = false, bool = false) {
    std::cout << ptr;
  }
  inline void log(void* ptr, bool = false, bool = false) {
    std::cout << ptr;
  }
  inline void log(int i, bool = false, bool = false) {
    std::cout << i;
  }
#endif

template<typename Type, typename ...Types>
void multi_log(Type First, Types... Others) {
  int count = sizeof...(Types);
  /// The first will have coordinates
  log(First, /*with_coord*/true, /*chained*/true);
  /// The others, if any,  will not have coordinates
  (log(Others, /*with_coord*/false, /*chained all but last*/--count), ...);
}

}

#if defined(__SYCL_DEVICE_ONLY__)
/// Notify the host that the kernel has finished.
__attribute__((noreturn)) void finish_kernel() {
  trisycl::vendor::xilinx::acap::aie::done_rpc::data_type dt{};
  /// Inform the host via RPC that the kernel has finished
  trisycl::vendor::xilinx::acap::aie::rpc::device_side::get()->perform(dt);
  trisycl::vendor::xilinx::acap::log("exiting\n");
  while (1)
    acap_intr::memory_fence();
}


/// The assert macro will call this function if an assertion fails on device.
void __assert_fail(const char *expr, const char *file, unsigned int line,
                   const char *func) {
  trisycl::vendor::xilinx::acap::multi_log("acap : ", file, ":", line, ": ",
                                           func, ": Assertion `", expr,
                                           "' failed\n", "kernel killed\n");
  finish_kernel();
}

#endif

#endif
