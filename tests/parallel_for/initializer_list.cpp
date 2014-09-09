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

    buffer<int,1> a(N);

    command_group { myQueue, [&] () {
        auto acc = a.get_access<access::write>();
        parallel_for({ N },
                     kernel_lambda<class nothing>([=] (id<1> index) {
                         acc[index] = index[0];
                       }));
      }};
    VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0]; });

    buffer<int,2> b({ N, 3 });

    command_group { myQueue, [&] () {
        auto acc = b.get_access<access::write>();
        parallel_for({ N, 3 },
                     kernel_lambda<class nothing>([=] (id<2> index) {
                         acc[index] = 10*index[1] + index[0];
                       }));
      }};
    VERIFY_BUFFER_VALUE(b, [](id<2> i) { return 10*i[1] + i[0]; });
  }
  return 0;
}
