#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEVICE_LIBSTDCPP_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_DEVICE_LIBSTDCPP_HPP

/** \file

    This contains many definition that allows using part of libstdc++ on device.
    With this header it is possible to use std::sting, std::vector and more...

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "device_allocator.hpp"
#include "log.hpp"
#include <cassert>
#include <new>

#ifdef __SYCL_DEVICE_ONLY__

/// All these functions are declared in the headers but only defined in the .so
/// file, so we provide their definition. Since they are all exception related we
/// assert because we do not have exception support on device.
namespace std {
void __throw_bad_exception(void) { assert(false && "cannot throw on device"); }
void __throw_bad_alloc(void) { assert(false && "cannot throw on device"); }
void __throw_bad_cast(void) { assert(false && "cannot throw on device"); }
void __throw_bad_typeid(void) { assert(false && "cannot throw on device"); }
void __throw_bad_array_new_length(void) {
  assert(false && "cannot throw on device");
}
void __throw_logic_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_domain_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_invalid_argument(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_length_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_out_of_range(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_out_of_range_fmt(const char *, ...)
    __attribute__((__format__(__gnu_printf__, 1, 2))) {
  assert(false && "cannot throw on device");
}
void __throw_runtime_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_range_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_overflow_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_underflow_error(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_ios_failure(const char *) {
  assert(false && "cannot throw on device");
}
void __throw_ios_failure(const char *, int) {
  assert(false && "cannot throw on device");
}
void __throw_system_error(int) { assert(false && "cannot throw on device"); }
void __throw_future_error(int) { assert(false && "cannot throw on device"); }
void __throw_bad_function_call() { assert(false && "cannot throw on device"); }
} // namespace std

/// Defined all possible variants of the operator new. Since we never throw but
/// assert on allocation failure, there is no difference between the throwing and
/// non-throwing version.
/// According to cppreference by default every implementation will use either of
/// these 2 new overload. So overloading only those 2 should be enough to
/// replace all of them.
void *operator new(std::size_t count) {
  return trisycl::vendor::xilinx::acap::heap::malloc(count);
}
void *operator new(std::size_t count, std::align_val_t al) {
  /// heap::malloc will always provide allocation aligned on at least
  /// heap::alloc_align
  assert(
      al <=
          (std::align_val_t)trisycl::vendor::xilinx::acap::heap::alloc_align &&
      "the allocator doesn't support higher alignments.");
  return trisycl::vendor::xilinx::acap::heap::malloc(count);
}

/// Same here according to cppreference all operator delete based on one of
/// those two.
void operator delete(void *ptr) noexcept {
  trisycl::vendor::xilinx::acap::heap::free(ptr);
}
void operator delete(void *ptr, std::align_val_t al) noexcept {
  trisycl::vendor::xilinx::acap::heap::free(ptr);
}

#endif

#endif
