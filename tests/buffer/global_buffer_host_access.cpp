// Check that a global buffer can be assigned and used from the host


/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1 2 3 4 5 6
*/
#include <CL/sycl.hpp>
#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace cl::sycl;

constexpr size_t N = 2;
constexpr size_t M = 3;
using Matrix = float[N][M];

buffer<float, 2> A;

int main() {
  Matrix a = { { 1, 2, 3 }, { 4, 5, 6 } };

  // Create buffers from a, b & c storage
  A = { &a[0][0], range<2> { N, M } };

  auto c = A.get_access<access::mode::read>();

  std::cout << std::endl << "Result:" << std::endl;
  for (size_t i = 0; i != N; i++)
    for (size_t j = 0; j != M; j++)
      std::cout << c[i][j] << " ";
  std::cout << std::endl;

  return 0;
}
