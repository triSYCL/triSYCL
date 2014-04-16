#include <CL/sycl.hpp>
#include <iostream>

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
    command_group (myQueue, [&] () {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::read>();
      auto kb = B.get_access<access::read>();
      auto kc = C.get_access<access::write>();

      // Enqueue a parallel kernel
      parallel_for(range<2> { N, M },
                   kernel_lambda<class matrix_add>([=] (id<2> index) {
        std::cout << index.get(0) << "," << index.get(1) << " ";
        kc[index] = ka[index] + kb[index];
      }));
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << std::endl << "Result:" << std::endl;
  for(int i = 0; i < N; i++)
    for(int j = 0; j < M; j++)
      std::cout << c[i][j] << " ";
  std::cout << std::endl;

  return 0;
}
