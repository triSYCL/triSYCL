/* Simple vector addition using SYCL in OpenCL interoperability mode
   Boost.Compute C++ host API and precompiled Vivado HLS C++ kernel.

   ap_int type in Vivado HLS arbitrary precision data types library
   is used to hold value in this example.

   Compared to using plain Boost.Compute OpenCL host API, in SYCL the
   buffer transfers between host and device are implicits with the
   accessors.
*/

/* REQUIRES: xilinx-xocc
   RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 6 8 11
*/
#include <iostream>
#include <iterator>
#include <array>
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

#include "ap_int.h"

using namespace cl::sycl;

constexpr size_t N = 3;

// Each element in Vector has an 256-bit signed integer data type
using Type = ap_int<256>;
using Vector = std::array<Type, N>;

int test_main(int argc, char *argv[]) {
  Vector a = { 1, 2, 3 };
  Vector b = { 5, 6, 8 };
  Vector c;

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
    boost::compute::program::create_with_binary_file(
      "hls_ap_vector_add_kernel.xclbin", opencl_q.get_context());

  // Build the OpenCL program
  program.build();

  // Build a SYCL kernel from the OpenCL kernel
  kernel k { boost::compute::kernel { program, "vector_add" } };


  // Create buffers from a & b vectors
  buffer<Type> A { std::begin(a), std::end(a) };
  buffer<Type> B { std::begin(b), std::end(b) };

  {
    // A buffer of N Type using the storage of c
    buffer<Type> C { c.data(), N };

    // Launch the vector parallel addition
    q.submit([&] (handler &cgh) {
        /* The host-device copies are managed transparently by these
           accessors: */
        cgh.set_args(A.get_access<access::mode::read>(cgh),
                     B.get_access<access::mode::read>(cgh),
                     C.get_access<access::mode::write>(cgh));
        /* For Vivado HLS C++ kernel, single_task is called here.
           Parallelism can be achieved by specifying pragma in Vivado HLS
           C++ kernel.*/
        cgh.single_task(k);
      }); //< End of our commands for this queue
  } //< Buffer C goes out of scope and copies back values to c

  std::cout << std::endl << "Result:" << std::endl;
  for (auto e : c)
    std::cout << e << " ";
  std::cout << std::endl;

  return 0;
}
