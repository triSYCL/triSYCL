/* RUN: %{execute}%s | %{filecheck} %s
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

  Matrix c;

  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // Create buffers from a, b & c storage
    buffer<float, 2> A(&a[0][0], range<2> { N, M });
    buffer<float, 2> B(&b[0][0], range<2> { N, M });
    buffer<float, 2> C(&c[0][0], range<2> { N, M });

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](handler &cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::mode::read>(cgh);
      auto kb = B.get_access<access::mode::read>(cgh);
      auto kc = C.get_access<access::mode::write>(cgh);

      // Enqueue a parallel kernel
      cgh.parallel_for<class matrix_add>(range<2> { N, M },
                                         [=] (id<2> index) {
        // Display the work-item coordinate during "kernel" execution
        std::cout << index.get(0) << "," << index.get(1) << " "
#ifdef _OPENMP
        /* Display also on which OpenMP thread a work-item is executed.
           Of course, the output may be a little-bit intermixed between
           threads according to OpenMP implementation...
         */
                     "(on thread " << omp_get_thread_num() << ") "
#endif
          ;
        // The real work is only this
        kc[index] = ka[index] + kb[index];
                                         });
      }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << std::endl << "Result:" << std::endl;
  for (size_t i = 0; i != N; i++)
    for (size_t j = 0; j != M; j++)
      std::cout << c[i][j] << " ";
  std::cout << std::endl;

  return 0;
}
