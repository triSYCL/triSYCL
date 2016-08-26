/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 6 8 11
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

int main() {
  Vector a = { 1, 2, 3 };
  Vector b = { 5, 6, 8 };
  Vector c;

  // Create a queue to work on
  queue q;

  // Create buffers from a & b vectors
  buffer<float> A { std::begin(a), std::end(a) };
  buffer<float> B { std::begin(b), std::end(b) };

  {
    // A buffer of N float using the storage of c
    buffer<float> C { c, N };

    /* The command group describing all operations needed for the kernel
       execution */
    q.submit([&](handler &cgh) {
        // In the kernel A and B are read, but C is written
        auto ka = A.get_access<access::mode::read>(cgh);
        auto kb = B.get_access<access::mode::read>(cgh);
        auto kc = C.get_access<access::mode::write>(cgh);

        // Enqueue a parallel kernel
        cgh.parallel_for<class vector_add>(N,
                                           [=] (id<1> index) {
                                             kc[index] = ka[index] + kb[index];
                                           });
      }); //< End of our commands for this queue
  } //< Buffer C goes out of scope and copies back values to c

  std::cout << std::endl << "Result:" << std::endl;
  for (auto e : c)
    std::cout << e << " ";
  std::cout << std::endl;
}
