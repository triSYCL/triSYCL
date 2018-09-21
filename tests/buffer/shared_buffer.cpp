/* RUN: %{execute}%s
   CHECK: buffer "a" is read_only: 0
   CHECK-NEXT: buffer "b" is read_only: 0
   CHECK-NEXT: buffer "c" is read_only: 1
   CHECK-NEXT: buffer "vb" is read_only: 0
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

// Size of the buffers
constexpr size_t N = 20;


// To verify it works through function return
cl::sycl::buffer<int, 1> f(void) {
  // Create a read-write 1D buffer of size N
  cl::sycl::buffer<int, 1> a(N);
  DISPLAY_BUFFER_USE_COUNT(a);
  DISPLAY_BUFFER_READ_ONLY_STATUS(a);

  {
    cl::sycl::buffer<int, 1> b { a };
    DISPLAY_BUFFER_USE_COUNT(a);
    DISPLAY_BUFFER_USE_COUNT(b);
    DISPLAY_BUFFER_READ_ONLY_STATUS(b);
    cl::sycl::buffer<int, 1> c = b;
    DISPLAY_BUFFER_USE_COUNT(a);
    DISPLAY_BUFFER_USE_COUNT(b);
    DISPLAY_BUFFER_USE_COUNT(c);
    auto B = b.get_access<access::mode::write>();

    for (std::size_t i = 0; i != N; ++i)
      B[i] = i*56 - 100;

    // Check b & c storage is really shared
    auto C = c.get_access<access::mode::read>();
    DISPLAY_BUFFER_USE_COUNT(a);
    DISPLAY_BUFFER_USE_COUNT(b);
    DISPLAY_BUFFER_USE_COUNT(c);
    DISPLAY_BUFFER_READ_ONLY_STATUS(c);
    VERIFY_BUFFER_VALUE(b, [] (id<1> i) { return i[0]*56 - 100; });

    b = c;
    DISPLAY_BUFFER_USE_COUNT(a);
    DISPLAY_BUFFER_USE_COUNT(b);
    DISPLAY_BUFFER_USE_COUNT(c);
  }
  DISPLAY_BUFFER_USE_COUNT(a);
  return a;
}

int main() {
  // Check that the r-value assignment/constructor works
  auto z = f();
  DISPLAY_BUFFER_USE_COUNT(z);
  DISPLAY_BUFFER_READ_ONLY_STATUS(z);
  return 0;
}
