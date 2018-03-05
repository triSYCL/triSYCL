/* Trivial kernel OpenCL interoperability
 */

/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet
 */
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  /* Construct the queue from the default OpenCL one.

     You can use the following environment variables to select
     the device to be chosen at runtime
     BOOST_COMPUTE_DEFAULT_DEVICE
     BOOST_COMPUTE_DEFAULT_DEVICE_TYPE
     BOOST_COMPUTE_DEFAULT_PLATFORM
     BOOST_COMPUTE_DEFAULT_VENDOR

     for example doing in bash
     export BOOST_COMPUTE_DEFAULT_PLATFORM=Xilinx
     will probably select for execution a Xilinx FPGA on the machine.

     export BOOST_COMPUTE_DEFAULT_PLATFORM="Portable Computing Language"
     will select PoCL.
  */
  queue q { default_selector { } };

  // Create output buffer
  buffer<int> output { 1 };

  q.submit([&] (handler &cgh) {
      auto a_output = output.get_access<access::mode::discard_write>(cgh);
      cgh.single_task([=,
                       a_output =
                       drt::accessor<decltype(a_output)> { a_output }] {
          a_output[0] = 42;
        });
    });

  auto a_output = output.get_access<access::mode::read>();
  BOOST_CHECK(a_output[0] == 42);

  return 0;
}
