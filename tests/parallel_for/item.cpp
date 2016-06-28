/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"
using namespace cl::sycl;

constexpr size_t N = 5;

int main() {
  {
    queue myQueue;


    buffer<unsigned int,1> a(N);

    myQueue.submit([&](handler &cgh) {
        auto acc = a.get_access<access::mode::write>(cgh);
        cgh.parallel_for<class nothing>(range<1> { N }, /* Offset */ id<1> { 7 },
                                        [=] (item<1> index) {
                                          acc[index[0] - 7] = index[0];
                                        });
      });
    VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0] + 7; });
  }
  return 0;
}
