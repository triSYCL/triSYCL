/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet

   A simple typical FPGA-like kernel adding 2 vectors
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

constexpr size_t N = 300;
using Type = int;

int test_main(int argc, char *argv[]) {
  buffer<Type> a { N };
  buffer<Type> b { N };
  buffer<Type> c { N };

  {
    auto a_b = b.get_access<access::mode::discard_write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_b.begin(), a_b.end(), 0);
  }

  {
    auto a_c = c.get_access<access::mode::discard_write>();
    // Initialize buffer with increasing numbers starting at 5
    std::iota(a_c.begin(), a_c.end(), 5);
  }

  /* Construct the queue from the default OpenCL one.

     You can use the following environment variables to select
     the device to be chosen at runtime
     BOOST_COMPUTE_DEFAULT_DEVICE
     BOOST_COMPUTE_DEFAULT_DEVICE_TYPE
     BOOST_COMPUTE_DEFAULT_PLATFORM
     BOOST_COMPUTE_DEFAULT_VENDOR

     for example doing in bash
     export BOOST_COMPUTE_DEFAULT_PLATFORM=Xilinx
     will probably select for execution a Xilinx FPGA on the machine

     export BOOST_COMPUTE_DEFAULT_PLATFORM="Portable Computing Language"
     will select PoCL. */
  queue q { default_selector {} };

  // Launch a kernel to do the summation
  q.submit([&] (handler &cgh) {
      // Get access to the data
      auto a_a = a.get_access<access::mode::discard_write>(cgh);
      auto a_b = b.get_access<access::mode::read>(cgh);
      auto a_c = c.get_access<access::mode::read>(cgh);

      // A typical FPGA-style pipelined kernel
      cgh.single_task<class add>([
          d_a = drt::accessor<decltype(a_a)> { a_a },
          d_b = drt::accessor<decltype(a_b)> { a_b },
          d_c = drt::accessor<decltype(a_c)> { a_c }] () {
          // Use an intermediate automatic array
          decltype(d_b)::value_type array[N];
          // This should not generate a call to
          // @llvm.memcpy.p0i8.p1i8.i64 in the SPIR output
          // because it makes argument promotion not working
          for (unsigned int i = 0 ; i < N; ++i)
            array[i] = d_b[i];
          for (unsigned int i = 0 ; i < N; ++i)
            d_a[i] = array[i] + d_c[i];
        });
    });

  // Verify the result
  auto a_a = a.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < a.get_count(); ++i)
    BOOST_CHECK(a_a[i] == 5 + 2*i);

  return 0;
}
