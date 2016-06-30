/* RUN: %{execute}%s
*/
#include <iostream>
#include <boost/compute.hpp>

#include <CL/sycl.hpp>

#include "test-helpers.hpp"

using namespace cl::sycl;

constexpr size_t N = 30;

int main() {
  // Construct an OpenCL program from the source string
  auto program = boost::compute::program::create_with_source(R"(
    __kernel void empty() {
    }
    )", boost::compute::system::default_context());
  // Build a kernel from the OpenCL kernel
  program.build();
  /* Get the OpenCL kernel as its own variable for extended lifetime
     so we do not have to have a clRetainKernel */
  auto bk = boost::compute::kernel { program, "empty" };
  kernel k { bk.get() };
  // Construct the queue from the defaul OpenCL one
  queue q { boost::compute::system::default_queue() };

#if 0
  // \todo add to spec section Defining kernels using OpenCL C kernel objects
  q.submit([&](handler &cgh) {
      cgh.parallel_for<class empty>(N, kernel);
    });

  // \todo add to spec section Defining kernels using OpenCL C kernel objects
  // The same with the equivalent host fallback
  q.submit([&](handler &cgh) {
      cgh.parallel_for<class empty>(N, kernel,
                                    // Host fallback
                                    [=] (int index) { }
                                    );
    });
#endif

  return 0;
}
