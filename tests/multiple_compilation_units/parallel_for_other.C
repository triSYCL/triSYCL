/* The OpenMP based barrier use nested parallelism that makes order of
   execution in parallel_for_workitem non deterministic so disable it on
   this test
*/
#define TRISYCL_NO_BARRIER

#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

void other() {
  Vector a = { 1, 2, 3 };
  Vector b = { 5, 6, 8 };
  float c[N];

  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // Create buffers from a & b vectors with 2 different syntax
    buffer<float> A (std::begin(a), std::end(a));
    buffer<float> B { std::begin(b), std::end(b) };

    // A buffer of N float using the storage of c
    buffer<float> C(c, N);

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](handler &cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::read>(cgh);
      auto kb = B.get_access<access::read>(cgh);
      auto kc = C.get_access<access::write>(cgh);

      // Enqueue a parallel kernel
      cgh.parallel_for<class vector_add>(range<1> { N },
                                         [=] (id<1> index) {
                                           std::cout << index.get(0) << " ";
                                           kc[index] = ka[index] + kb[index];
                                         });
      }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << std::endl << "Result:" << std::endl;
  for (int i = 0; i != N; i++)
    std::cout << c[i] << " ";
  std::cout << std::endl;
}
