/* Simple OpenCL vector addition using SYCL in OpenCL interoperability
   mode Boost.Compute C++ host API and precompiled kernel.

   Check that we can pass a scalar to a kernel.

   Compared to using plain Boost.Compute OpenCL host API, in SYCL the
   buffer transfers between host and device are implicits with the
   accessors.
*/

/* RUN: %{execute}%s
*/
#include <iostream>
#include <iterator>
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

using namespace cl::sycl;

using Type = int;

constexpr size_t N = 300;
constexpr Type offset = 33;

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
  queue q { default_selector { } };

  // Construct an OpenCL program from the precompiled kernel file
  auto program = boost::compute::program::create_with_binary_file
    ("vector_add_int.spir64", q.get_boost_compute().get_context());

  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  kernel k { boost::compute::kernel { program, "vector_add_int" } };

  {
    // Launch the vector parallel addition
    q.submit([&] (handler &cgh) {
        /* The host-device copies are managed transparently by these
           accessors: */
        cgh.set_args(input.get_access<access::mode::read>(cgh),
                     output.get_access<access::mode::write>(cgh),
                     // To be sure to use the \c int type on the kernel side
                     cl::sycl::cl_int { offset });
        // Launch the kernel
        cgh.single_task(k);
      }); //< End of our commands for this queue
  }

  // Verify the result
  auto a_input = input.get_access<access::mode::read>();
  auto a_output = output.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < input.get_count(); ++i)
    BOOST_CHECK(a_output[i] == a_input[i] + offset);

  return 0;
}
