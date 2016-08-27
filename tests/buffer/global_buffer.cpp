/* Test computation with global buffers

   RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 3 5 7 9 11 13
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

// Test with global default buffers
buffer<float, 2> A;
buffer<float, 2> B;
buffer<float, 2> C;

int main() {
  Matrix a = { { 1, 2, 3 }, { 4, 5, 6 } };
  Matrix b = { { 2, 3, 4 }, { 5, 6, 7 } };

  Matrix c = { { 0 }, { 0 } };

  // Create a queue to work on
  queue q;

  /* Create *new* buffers from a, b & c storage

     So from here the previous default buffers are discarded and
     replaced by the new one
  */
  A = { &a[0][0], range<2> { N, M } };
  B = { &b[0][0], range<2> { N, M } };
  C = { &c[0][0], range<2> { N, M } };

  /* The command group describing all operations needed for the kernel
     execution */
  q.submit([&](handler &cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::mode::read>(cgh);
      auto kb = B.get_access<access::mode::read>(cgh);
      auto kc = C.get_access<access::mode::write>(cgh);

      // Enqueue a parallel kernel
      cgh.parallel_for<class matrix_add>(range<2> { N, M },
                                         [=] (id<2> index) {
                                           // The real work is only this
                                           kc[index] = ka[index] + kb[index];
                                         });
    });

  /* Be careful with global buffers with copy back behaviour, since
     the destructor is called at program exit (typically in some C++
     library at_exit handler at a time the object to copy to may be
     dead for a while, so with a lot of havoc and hard debug time...

     So since we want the buffer destructor of C to copy back the
     content just before using c, just reassign an empty buffer to C,
     it will call the destructor of the previous buffer content
  */
  C = {};

  std::cout << std::endl << "Result:" << std::endl;
  for (size_t i = 0; i != N; i++)
    for (size_t j = 0; j != M; j++)
      std::cout << c[i][j] << " ";
  std::cout << std::endl;

  return 0;
}
