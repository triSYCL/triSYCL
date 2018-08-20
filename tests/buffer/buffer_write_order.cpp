/* RUN: %{execute}%s

   Test to check if a deadlock happens when accessing the same buffer in read
   and write mode consecutively. Check \c add_buffer in \c task.hpp for more
   details
*/

#include <CL/sycl.hpp>

#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

int main() {
  queue q;

  buffer<int> A(N);

  q.submit([&] (handler &cgh) {
      auto ka = A.get_access<access::mode::write>(cgh);
      auto stub = A.get_access<access::mode::read>(cgh);
      cgh.parallel_for<>(N, [=] (id<1> index) {
          ka[index] = index[0];
        });
    });
  q.wait();
  return 0;
}
