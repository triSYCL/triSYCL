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
  /// This will send a null terminated string throught the RPC system to the host 
  void log_internal(const char *ptr) {
    aie::send_log_rpc::data_type sld;
    sld.data = ptr;
    sld.size = hw::strlen(ptr);
    acap::aie::rpc::device_side::get()->perform(sld);
  }

  /// This will log the tile coordinates that the null-terminate string.
  __attribute__((noinline)) void log(const char *ptr, bool with_coord = true) {
    if (with_coord) {
      char arr[] = "00, 0 : ";
      arr[0] = '0' + (hw::get_tile_x_coordinate() / 10);
      arr[1] = '0' + (hw::get_tile_x_coordinate() % 10);
      arr[4] = '0' + (hw::get_tile_y_coordinate() % 10);

      log_internal(arr);
    }
    log_internal(ptr);
  }

  /// This function serializes a number into a buffer.
  static void write_number(auto write, int i, const char* base_char = "0123456789") {
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
      write(base_char[std::abs((i / hw::pow(base, d - 1)) % base)]);
  }

  /// This __attribute__((noinline)) is just to make the IR more readable. it is
  /// not required for any other reason.
  __attribute__((noinline)) void log(int i, bool with_coord = true, const char* base = "0123456789") {
    char arr[/*bits in base 2*/31 + /*sign*/1 + /*\0*/1];
    char *ptr = &arr[0];
    write_number([&](char c) mutable { *(ptr++) = c; }, i, base);
    ptr[0] = '\0';
    log(arr, with_coord);
  }
  /// This __attribute__((noinline)) is just to make the IR more readable. it is
  /// not required for any other reason.
  __attribute__((noinline)) void log(void* p, bool with_coord = true) {
    log("0x", with_coord);
    log((int)p, false, "0123456789abcdef");
  }
#else
  void log(const char* ptr) {
    std::cout << ptr;
  }
  void log(void* ptr) {
    std::cout << ptr;
  }
  void log(int i) {
    std::cout << i;
  }
#endif

}

#if defined(__SYCL_DEVICE_ONLY__)
/// The annotation here is to prevent the global variable remover from removing
/// this function.
__attribute__((annotate("used"))) __attribute__((noreturn)) void finish_kernel() {
  trisycl::vendor::xilinx::acap::aie::done_rpc::data_type dt{};
  trisycl::vendor::xilinx::acap::aie::rpc::device_side::get()->perform(dt);
  trisycl::vendor::xilinx::acap::log("exiting");
  acap_intr::core_done();
}

void __assert_fail(const char *expr, const char *file,
                              unsigned int line, const char *func) {
  using trisycl::vendor::xilinx::acap::log;
  log("acap : ", /*with_coord*/true);
  log(file, /*with_coord*/false);
  log(":", /*with_coord*/false);
  log(line, /*with_coord*/false);
  log(": ", /*with_coord*/false);
  log(func, /*with_coord*/false);
  log(": Assertion `", /*with_coord*/false);
  log(expr, /*with_coord*/false);
  log("' failed\n", /*with_coord*/false);
  log("kernel killed\n", /*with_coord*/false);
  finish_kernel();
}

#endif

#endif
