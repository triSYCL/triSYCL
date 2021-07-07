/* RUN: %{execute}%s

   Verify that a host kernel can modify a global variable

   Example inspired by Morris Hafner @ Codeplay

   Related to committee discussion
   https://gitlab.khronos.org/sycl/Specification/issues/168
*/
#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

int global = 0;

struct increment_global;

int test_main(int argc, char *argv[]) {
  cl::sycl::queue q{cl::sycl::host_selector {}};
  // In the following we do not need to capture anything since we have
  // only a global variable
  q.submit([] (cl::sycl::handler &cgh) {
    cgh.single_task<increment_global>([] {
      ++global;
    });
  });
  q.wait();

  BOOST_CHECK(global == 1);

  return 0;
}
