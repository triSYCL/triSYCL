/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet

   A simple typical FPGA-like kernel

   Replace a loop by some explicit iteration instances to debug loop
   issues.

   With no loop at all is it better to test if the basic runtime works
   on moving buffers around.
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>
#include <numeric>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

constexpr size_t N = 7;
using Type = int;


int test_main(int argc, char *argv[]) {
  Type initial_values[N] = { 1, 2, 3, 4, 5, 6, 7 };
  buffer<Type> input { std::cbegin(initial_values),
                       std::cend(initial_values) };
  buffer<Type> output { N };

  /* Construct the queue from the default OpenCL one.

     You can use the following environment variables to select
     the device to be chosen at runtime
     BOOST_COMPUTE_DEFAULT_DEVICE
     BOOST_COMPUTE_DEFAULT_DEVICE_TYPE
     BOOST_COMPUTE_DEFAULT_ENFORCE
     BOOST_COMPUTE_DEFAULT_PLATFORM
     BOOST_COMPUTE_DEFAULT_VENDOR

     for example doing in bash
     export BOOST_COMPUTE_DEFAULT_VENDOR=Xilinx
     export BOOST_COMPUTE_DEFAULT_ENFORCE=1
     will select for execution a Xilinx FPGA on the machine
  */
  queue q { default_selector {} };

  // Launch a kernel to do the summation
  q.submit([&] (handler &cgh) {
      // Get access to the data
      auto a_input = input.get_access<access::mode::read>(cgh);
      auto x_output = output.get_access<access::mode::write>(cgh);

      // A typical FPGA-style pipelined kernel
      cgh.single_task<class add>([=,
                                  x_output = drt::accessor<decltype(x_output)> { x_output },
                                  a_input = drt::accessor<decltype(a_input)> { a_input }] {
                                   /* Use extensive iteration to test
                                      computation outside of loops and
                                      in some order to avoid the
                                      compiler to reloop the
                                      computaion */
#define INC(i) x_output[i] = a_input[i] + 42;
                                   INC(4);
                                   INC(6);
                                   INC(5);
                                   INC(0);
                                   INC(3);
                                   INC(2);
                                   INC(1);
        });
    });

  // Verify the result
  auto a_input = input.get_access<access::mode::read>();
  auto a_output = output.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < input.get_count(); ++i)
    BOOST_CHECK(a_output[i] == a_input[i] + 42);

  return 0;
}
