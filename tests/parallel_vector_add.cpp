#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

int main() {
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
    command_group (myQueue, [&] () {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<access::read>();
      auto kb = B.get_access<access::read>();
      auto kc = C.get_access<access::write>();

      // Enqueue a single, simple task
      parallel_for(N, kernel_lambda<class vector_add>([=] (id index) {
            std::cout << std::get<0>(index) << " ";
            kc[index] = ka[index] + kb[index];
      }));
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << "Result:" << std::endl;
  for(int i = 0; i < N; i++)
    std::cout << c[i] << " ";
  std::cout << std::endl;

  return 0;
}
