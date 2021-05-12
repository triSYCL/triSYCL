/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 6 8 11
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>

constexpr size_t N = 3;
using Vector = float[N];

// A static-scoped pipe of 4 float elements
cl::sycl::sycl_2_2::static_pipe<float, 4> p;

int main() {
  Vector va = { 1, 2, 3 };
  Vector vb = { 5, 6, 8 };
  Vector vc;

  {
    // Create buffers from a & b vectors
    cl::sycl::buffer<float> ba { std::begin(va), std::end(va) };
    cl::sycl::buffer<float> bb { std::begin(vb), std::end(vb) };

    // A buffer of N float using the storage of vc
    cl::sycl::buffer<float> bc { vc, N };

    // Create a queue to launch the kernels
    cl::sycl::queue q;

    // Launch the producer to stream A to the pipe
    q.submit([&](cl::sycl::handler &cgh) {
      // Get write access to the pipe
      auto kp = p.get_access<cl::sycl::access::mode::write>(cgh);
      // Get read access to the data
      auto ka = ba.get_access<cl::sycl::access::mode::read>(cgh);

      cgh.single_task<class producer>([=] {
          for (int i = 0; i != N; i++)
            // Try to write to the pipe up to success
            while (!(kp.write(ka[i])))
              ;
        });
      });

    // Launch the consumer that adds the pipe stream with B to C
    q.submit([&](cl::sycl::handler &cgh) {
      // Get read access to the pipe
      auto kp = p.get_access<cl::sycl::access::mode::read>(cgh);

      // Get access to the input/output buffers
      auto kb = bb.get_access<cl::sycl::access::mode::read>(cgh);
      auto kc = bc.get_access<cl::sycl::access::mode::write>(cgh);

      cgh.single_task<class consumer>([=] {
          for (int i = 0; i != N; i++) {
            /* Declare a variable of the same type as what the pipe
               can deal (a good example of single source advantage)
            */
            decltype(kp)::value_type e;
            // Try to read from the pipe up to success
            while (!(kp.read(e)))
              ;
            kc[i] = e + kb[i];
          }
        });
      });
  } //< End scope for the queue and the buffers, so wait for completion

  std::cout << std::endl << "Result:" << std::endl;
  for (auto e : vc)
    std::cout << e << " ";
  std::cout << std::endl;
}
