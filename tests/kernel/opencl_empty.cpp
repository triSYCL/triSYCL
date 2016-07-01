/* RUN: %{execute}%s
*/
#include <iostream>
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

constexpr size_t N = 30;

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

  // Verify that the cache works when asking for same kernel
  kernel k2 { bk.get() };
  BOOST_CHECK(k == k2);

  /* Do not use the default assignment check check_assignment() since
     there is no default constructor. Use an explicit check */
  k2 = k;
  BOOST_CHECK(k == k2);

  // The other usual checks
  check_copy(k);
  check_associative_ordered_container(k);
  check_associative_unordered_container(k);

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
