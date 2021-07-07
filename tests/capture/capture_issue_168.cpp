/* RUN: %{execute}%s

   Experiment with example inspired by Morris Hafner @ Codeplay
   https://gitlab.khronos.org/sycl/Specification/issues/168
*/
#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

struct increment_local;
struct increment_global;

int global = 0;

int test_main(int argc, char *argv[]) {
  cl::sycl::queue q;

  int local = 0;
  q.submit([&](cl::sycl::handler &cgh) {
    cgh.single_task<increment_local>([=]() mutable {
      ++local;
    });
  });

  q.submit([&](cl::sycl::handler &cgh) {
    cgh.single_task<increment_global>([=]() {
      ++global;
    });
  });
  q.wait();

  // local is still 0
  BOOST_CHECK(local == 0);

  // Would be 1 with host device
  // Would be 0 with OpenCL device
  BOOST_CHECK(global == 1);

  return 0;
}
