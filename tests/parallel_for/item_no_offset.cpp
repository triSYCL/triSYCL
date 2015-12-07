/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 30;

int main() {
  {
    queue myQueue;

    buffer<unsigned int,1> a(range<1>{N});
    myQueue.submit([&](handler &cgh) {
        auto acc = a.get_access<access::mode::write>(cgh);
        cgh.parallel_for<class nothing>(range<1>{N}, [=] (item<1> index) {
//            acc[index.get_linear_id()] = index.get_linear_id();
            acc[index[0]] = index[0];
          });
      });
    // Verify that a[i] == i
    VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0]; });
  }
  return 0;
}
