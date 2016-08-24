/* Wrong example with new/delete of accessors

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

int main() {
  Matrix a = { { 1, 2, 3 }, { 4, 5, 6 } };
  Matrix b = { { 2, 3, 4 }, { 5, 6, 7 } };

  Matrix c = { { 0 }, { 0 } };

  {
    buffer<float, 2> A = { &a[0][0], range<2> { N, M } };
    buffer<float, 2> B = { &b[0][0], range<2> { N, M } };
    buffer<float, 2> C = { &c[0][0], range<2> { N, M } };

    /* The command group describing all operations needed for the
       kernel execution */
    queue {}.submit([&](handler &cgh) {
        // In the kernel A and B are read, but C is written
        auto ka = A.get_access<access::mode::read>(cgh);
        auto kb = B.get_access<access::mode::read>(cgh);
        auto kc = C.get_access<access::mode::write>(cgh);

        auto kap = new accessor<float, 2, access::mode::read> { ka };
        auto kbp = new accessor<float, 2, access::mode::read> { kb };
        auto kcp = new accessor<float, 2, access::mode::write> { kc };
        /** Enqueue a parallel kernel

            This cannot work because the parallel for is executed in
            asynchronously, with the delete at the end already
            executed...
        */
        cgh.parallel_for<class matrix_add>(range<2> { N, M },
                                           [=] (id<2> index) {
                                             // The real work is only this
                                             (*kcp)[index] =
                                               (*kap)[index] + (*kbp)[index];
                                           });
        delete kap;
        delete kbp;
        delete kcp;
      });
  } //< Copy-back of buffer C here

  std::cout << std::endl << "Result:" << std::endl;
  for (size_t i = 0; i != N; i++)
    for (size_t j = 0; j != M; j++)
      std::cout << c[i][j] << " ";
  std::cout << std::endl;

  return 0;
}
