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
        cgh.parallel_for<class nothing>(N,
                                        [=] (id<1> index) {
                                          acc[index] = index[0];
                                        });
      });
    VERIFY_BUFFER_VALUE(a, [](id<1> i) { return i[0]; });

    buffer<unsigned int,2> b({ N, 3 });

    myQueue.submit([&](handler &cgh) {
        auto acc = b.get_access<access::mode::write>(cgh);
        cgh.parallel_for<class nothing>({ N, 3 },
                                        [=] (id<2> index) {
                                          acc[index] = 10*index[1] + index[0];
                                        });
      });
    VERIFY_BUFFER_VALUE(b, [](id<2> i) { return 10*i[1] + i[0]; });

#if 0
    command_group { myQueue, [&] () {
        auto acc = b.get_access<access::mode::write>(cgh);
        parallel_for<class nothing>({ N }, { 3 },
                                    [=] (item<1> index) {
                                      acc[index] = 2*index.get_global()[1]
                                        + index.get_global()[0];
                                    });
      }};
    VERIFY_BUFFER_VALUE(b, [](id<2> i) { return 2*i[1] + i[0]; });

    command_group { myQueue, [&] () {
        auto acc = b.get_access<access::mode::write>(cgh);
        parallel_for<class nothing>(N, { 3 },
                                    [=] (item<1> index) {
                                      std::cout << index.get_global()[0]
                                                << std::endl;
                                    });
      }};
    VERIFY_BUFFER_VALUE(b, [](id<2> i) { return 2*i[1] + i[0]; });

    command_group { myQueue, [&] () {
        auto acc = b.get_access<access::mode::write>(cgh);
        parallel_for<class nothing>({ N }, 3,
                                    [=] (item<1> index) {
            std::cout << index.get_global(0) << std::endl;
          });
      }};

    command_group { myQueue, [&] () {
        auto acc = b.get_access<access::mode::write>(cgh);
        parallel_for<class nothing>(N, 3,
                                    [=] (item<1> index) {
                                      std::cout << index.get_global(0)
                                                << std::endl;
                                    });
      }};

    command_group { myQueue, [&] () {
        auto acc = b.get_access<access::mode::write>(cgh);
        parallel_for<class nothing>(range<>(N), { 2 },
                     [=] (item<1> index) {
                         std::cout << index.get_global(0) << std::endl;
                     });
      }};
#endif
  }
  return 0;
}
