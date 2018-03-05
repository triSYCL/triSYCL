/* Simple OpenCL vector addition using SYCL in OpenCL interoperability
   mode Boost.Compute C++ host API and precompiled kernel.

   Compared to using plain Boost.Compute OpenCL host API, in SYCL the
   buffer transfers between host and device are implicits with the
   accessors.
*/

/* RUN: %{execute}%s
   REQUIRES: this-test-does-not-run-with-lit-yet

*/
#include <iostream>
#include <iterator>
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

using namespace cl::sycl;

constexpr size_t N = 300;
using Type = int;

int test_main(int argc, char *argv[]) {
  buffer<Type> input { N };
  buffer<Type> output { N };

  {
    auto a_input = input.get_access<access::mode::write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_input.begin(), a_input.end(), 0);
  }

  /* Construct the queue from the default OpenCL one.

     You can use the following environment variables to select
     the device to be chosen at runtime
     BOOST_COMPUTE_DEFAULT_DEVICE
     BOOST_COMPUTE_DEFAULT_DEVICE_TYPE
     BOOST_COMPUTE_DEFAULT_PLATFORM
     BOOST_COMPUTE_DEFAULT_VENDOR

     for example doing in bash
     export BOOST_COMPUTE_DEFAULT_VENDOR=Xilinx
     will probably select for execution a Xilinx FPGA on the machine
  */
  auto opencl_q { boost::compute::system::default_queue() };
  queue q { opencl_q };

  // Construct an OpenCL program from the precompiled kernel file
  auto program =
    boost::compute::program::create_with_binary_file("add_42.spir64",
                                                     opencl_q.get_context());

  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  kernel k { boost::compute::kernel { program, "_ZN2cl4sycl6detail18instantiate_kernelIZZ9test_mainiPPcENK3$_1clERNS0_7handlerEE3addZZ9test_mainiS4_ENKS5_clES7_EUlvE_EEvT0_" } };

  // Launch a kernel to do the summation
  q.submit([&] (handler &cgh) {
      // Get access to the data
      /* auto a_input = input.get_access<access::mode::read>(cgh);
         auto x_output = output.get_access<access::mode::write>(cgh);
      */
      /* The host-device copies are managed transparently by these
         accessors: */
      cgh.set_args(input.get_access<access::mode::read>(cgh),
                   output.get_access<access::mode::write>(cgh));
      // Launch the kernel
      cgh.single_task(k);
    });

  // Verify the result
  auto a_input = input.get_access<access::mode::read>();
  auto a_output = output.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < input.get_count(); ++i)
    BOOST_CHECK(a_output[i] == a_input[i] + 42);

  return 0;
}
