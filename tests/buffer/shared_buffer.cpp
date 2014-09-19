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


int main() {
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
    auto B = b.get_access<access::write, access::host_buffer>();

    for (std::size_t i = 0; i != N; ++i)
      B[i] = i;
    auto C = c.get_access<access::read, access::host_buffer>();
    DISPLAY_BUFFER_USE_COUNT(a);
    DISPLAY_BUFFER_USE_COUNT(b);
    DISPLAY_BUFFER_USE_COUNT(c);
    DISPLAY_BUFFER_READ_ONLY_STATUS(c);
  }
  DISPLAY_BUFFER_USE_COUNT(a);
  return 0;
}

