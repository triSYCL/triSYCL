/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 6 8 11

   Extend a pipe program example from Andrew Richards
   https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14215
   and use cool stream syntax
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

  {
    // Create buffers from a & b vectors
    cl::sycl::buffer<float> A { std::begin(a), std::end(a) };
    cl::sycl::buffer<float> B { std::begin(b), std::end(b) };

    // A buffer of N float using the storage of c
    cl::sycl::buffer<float> C { c, N };

    // A pipe of 1 float elements
    cl::sycl::sycl_2_2::pipe<float> P { 1 };

    // Create a queue to launch the kernels
    cl::sycl::queue q;

    // Launch the producer to stream A to the pipe
    q.submit([&](cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto p = P.get_access<cl::sycl::access::mode::write>(cgh);
      // Get read access to the data
      auto ka = A.get_access<cl::sycl::access::mode::read>(cgh);

      cgh.single_task<class producer>([=] {
          for (int i = 0; i != N; i++)
            // Try to write to the pipe up to success
            while (!(p << ka[i]))
              ;
        });
      });

    // Launch the consumer that adds the pipe stream with B to C
    q.submit([&](cl::sycl::handler &cgh) {
      // Get read access to the pipe
      auto p = P.get_access<cl::sycl::access::mode::read>(cgh);

      // Get access to the input/output buffers
      auto kb = B.get_access<cl::sycl::access::mode::read>(cgh);
      auto kc = C.get_access<cl::sycl::access::mode::write>(cgh);

      cgh.single_task<class consumer>([=] {
          for (int i = 0; i != N; i++) {
            /* Declare a variable of the same type as what the pipe
               can deal (a good example of single source advantage)
            */
            decltype(p)::value_type e;
            // Try to read from the pipe up to success
            while (!(p >> e))
              ;
            kc[i] = e + kb[i];
          }
        });
      });
  } //< End scope for the queue and the buffers, so wait for completion

  std::cout << std::endl << "Result:" << std::endl;
  for (auto e : c)
    std::cout << e << " ";
  std::cout << std::endl;
}
