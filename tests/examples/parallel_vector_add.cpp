/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 6 8 11
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>

constexpr size_t N = 3;
using Vector = float[N];

int main() {
  Vector a = { 1, 2, 3 };
  Vector b = { 5, 6, 8 };
  Vector c;

  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    cl::sycl::queue myQueue;

    // Create buffers from a & b vectors
    cl::sycl::buffer<float> A { std::begin(a), std::end(a) };
    cl::sycl::buffer<float> B { std::begin(b), std::end(b) };

    // A buffer of N float using the storage of c
    cl::sycl::buffer<float> C { c, N };

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](cl::sycl::handler &cgh) {
      // In the kernel A and B are read, but C is written
      auto ka = A.get_access<cl::sycl::access::mode::read>(cgh);
      auto kb = B.get_access<cl::sycl::access::mode::read>(cgh);
      auto kc = C.get_access<cl::sycl::access::mode::write>(cgh);

      // Enqueue a parallel kernel
      cgh.parallel_for<class vector_add>(cl::sycl::range<1> { N },
                                         [=] (cl::sycl::id<1> index) {
                                           kc[index] = ka[index] + kb[index];
                                         });
      }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  std::cout << std::endl << "Result:" << std::endl;
  for (auto e : c)
    std::cout << e << " ";
  std::cout << std::endl;
}
