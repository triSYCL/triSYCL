/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: buffer "a" is read_only: 0
   CHECK-NEXT: buffer "b" is read_only: 0
   CHECK-NEXT: buffer "c" is read_only: 0
   CHECK-NEXT: buffer "cc" is read_only: 1
   CHECK-NEXT: buffer "vb" is read_only: 0
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

// Size of the buffers
constexpr size_t N = 20;
constexpr size_t M = 30;
constexpr size_t P = 40;

int main() {
  // Create some buffers for each case of the specification

  // Create a read-write 1D buffer of size N
  cl::sycl::buffer<int, 1> a(N);
  DISPLAY_BUFFER_READ_ONLY_STATUS(a);

  // Create a read-write 2D buffer of size N*M upon host storage
  double array[N][M];
  cl::sycl::buffer<double, 2> b(&array[0][0], make_range(N, M));
  DISPLAY_BUFFER_READ_ONLY_STATUS(b);

  // Create a read-write 2D buffer of size N*M upon host storage
  const double carray[N][M] = { { 0 } };
  cl::sycl::buffer<double, 2> c(&carray[0][0], make_range(N, M));
  DISPLAY_BUFFER_READ_ONLY_STATUS(c);

  // Create a read-only 2D buffer of size N*M upon host storage
  const double ccarray[N][M] = { { 0 } };
  cl::sycl::buffer<const double, 2> cc { &ccarray[0][0], make_range(N, M) };
  DISPLAY_BUFFER_READ_ONLY_STATUS(cc);

  // Create a read-write 1D buffer initialized with copy of elements given
  // by a range. Since it is a copy, we can use another
  // (conversion-compatible) element type
  std::vector<int> v { 1, 8, 11 };
  cl::sycl::buffer<double, 1> vb(v.begin(), v.end());
  DISPLAY_BUFFER_READ_ONLY_STATUS(vb);

  return 0;
}
