/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

int main() {
  constexpr size_t N = 30;

  {
    queue myQueue;

    buffer<unsigned int,1> a(N);
    myQueue.submit([&](handler &cgh) {
        auto acc = a.get_access<access::mode::write>(cgh);
        // Show that we can use a simple parallel_for with int, for example
        cgh.parallel_for<class nothing>(N, [=] (int index) {
            acc[index] = index;
          });
      });
    // Verify that a[i] == i
    VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0]; });
  }
  return 0;
}
