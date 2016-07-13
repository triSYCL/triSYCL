/* RUN: %{execute}%s
*/
#include <iostream>
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
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

  // \todo add to spec section Defining kernels using OpenCL C kernel objects
  q.submit([&](handler &cgh) {
      cgh.parallel_for(43, k);
    });

  /* Since the kernel does not use any buffer, wait for its completion
     to be sure it is executed before the program exit */
  q.wait();

#if 0
  // \todo add to spec section Defining kernels using OpenCL C kernel objects
  // The same with the equivalent host fallback
  q.submit([&](handler &cgh) {
      cgh.parallel_for<class empty>(N, k,
                                    // Host fallback
                                    [=] (int index) { }
                                    );
    });
#endif

  return 0;
}
