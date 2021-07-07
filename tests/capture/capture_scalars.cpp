/* RUN: %{execute}%s

   Simple parallel for with some external scalars that are captured
*/
#include <CL/sycl.hpp>
#include <iostream>
#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 30;

int main() {
  queue q;

  buffer<unsigned int,1> a { N };
  int off { 123 };
  float off_float { 1.5F };
  q.submit([&] (handler &cgh) {
      auto acc = a.get_access<access::mode::write>(cgh);
      // Show that we can use a simple parallel_for with id<1>, for example
      cgh.parallel_for<class nothing>(N, [=] (id<1> index) {
          acc[index] = index[0] + off*off_float;
        });
    });
  // Verify the computation
  VERIFY_BUFFER_VALUE(a,
                      [=](id<1> i) -> decltype(a)::value_type
                      {
                        return i[0] + off*off_float;
                      });

  return 0;
}
