/* RUN: %{execute}%s

   Generalize the dimension number and use modern C++ features like
   - tuple-interface to id and range;
   - C++23 mdspan addressing.
*/
#include <iostream>

// Allows also ranks higher than 3
#define TRISYCL_ALLOW_ANY_DIMENSION

#include "sycl/sycl.hpp"
#include "test-helpers.hpp"

int main() {
  {
    sycl::queue q;

    sycl::buffer<int, 4> a { { 2, 3, 5, 7 } };

    q.submit([&](sycl::handler& cgh) {
      sycl::accessor acc { a, cgh };
      cgh.parallel_for(a.get_range(), [=](sycl::id<4> i) {
        auto [x, y, z, t] = i;
        acc[x, y, z, t] = x + y + z + t;
      });
    });
    VERIFY_BUFFER_VALUE(a, [](sycl::id<4> i) { return i[0] + i[1] + i[2] + i[3]; });
  return 0;
  }
}
