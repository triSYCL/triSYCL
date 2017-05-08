#include <CL/sycl.hpp>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

void other(Vector &a, Vector &b, Vector &c) {
  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // Create buffers from a & b vectors with 2 different syntax
    buffer<float> A (std::begin(a), std::end(a));
    buffer<float> B { std::begin(b), std::end(b) };

    // A buffer of N float using the storage of c
    buffer<float> C { c, N };

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](handler &cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::mode::read>(cgh);
      auto kb = B.get_access<access::mode::read>(cgh);
      auto kc = C.get_access<access::mode::discard_write>(cgh);

      // Enqueue a parallel kernel
      cgh.parallel_for<class vector_add>(range<1> { N },
                                         [=] (id<1> index) {
                                           kc[index] = ka[index] + kb[index];
                                         });
      }); // End of our commands for this queue
  } // End scope, so we wait for the C buffer write-back
}
