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
        if (index[0] == 0) {
          int i = 3;
          generic<int *> p {&i};
          generic<int *> p2 = &i;
          p = &i;
          std::cout << "i = " << *p << std::endl;
          float f[2] = { 2, 3 };
          generic<float *> q;
          q = f;
          std::cout << "f[0] = " << *q << std::endl;
          std::cout << "f[0] = " << q[0] << std::endl;
          std::cout << "f[1] = " << q[1] << std::endl;
          q++;
          std::cout << "f[1] = " << *q << std::endl;
          std::cout << "f[0] = " << q[-1] << std::endl;
          double d = 3;
          priv<double*> pd { &d };
          priv<double*> pd2 = &d;
          pd2 = &d;
        }
      }));
    }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << "Result:" << std::endl;
  for(int i = 0; i < N; i++)
    ;// std::cout << c[i] << " ";
  std::cout << std::endl;

  return 0;
}
