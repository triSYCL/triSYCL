#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];


int main() {

  float c[N];

  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // A buffer of N float using the storage of c
    buffer<float> C(c, N);

    /* The command group describing all operations needed for the kernel
       execution */
    command_group (myQueue, [&] {
      auto kc = C.get_access<access::write>();

      parallel_for(range<1> { N },
                   kernel_lambda<class generate>([=] (id<1> index) {
                       generic<int *> p;
      }));
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << "Result:" << std::endl;
  for(int i = 0; i < N; i++)
    ;// std::cout << c[i] << " ";
  std::cout << std::endl;

  return 0;
}
