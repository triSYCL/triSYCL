/* Trivial OpenCL kernel OpenCL interoperability
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
     export BOOST_COMPUTE_DEFAULT_VENDOR=Xilinx
     will probably select for execution a Xilinx FPGA on the machine
  */
  auto opencl_q { boost::compute::system::default_queue() };
  queue q { opencl_q };

  // Construct an OpenCL program from the precompiled kernel file
  auto program =
    boost::compute::program::create_with_binary_file("kernel_output_42.kernel.bc",
                                                     opencl_q.get_context());

  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  kernel k { boost::compute::kernel { program, "_ZN2cl4sycl6detail18instantiate_kernelIDnZZ9test_mainiPPcENK3$_1clERNS0_7handlerEEUlvE_EEvT0_" } };


  // Create output buffer
  buffer<int> output { 1 };

  q.submit([&] (handler &cgh) {
      cgh.set_args(output.get_access<access::mode::discard_write>(cgh));
      cgh.single_task(k);
    });

  auto a_output = output.get_access<access::mode::read>();
  BOOST_CHECK(a_output[0] == 42);

  return 0;
}
