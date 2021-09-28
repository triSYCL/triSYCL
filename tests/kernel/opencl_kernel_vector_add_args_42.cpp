/* RUN: %{execute}%s

   Run an OpenCL kernel adding 2 vectors and a scalar
*/
#include <iostream>
#include <iterator>

#include <CL/sycl.hpp>

#include <boost/compute.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

TEST_CASE("OpenCL kernel adding 2 vectors and a scalar", "[opencl_kernel]") {
  Vector a = { 1, 2, 3 };
  Vector b = { 5, 6, 8 };
  Vector c;

  // Construct the queue from the defaul OpenCL one
  queue q { boost::compute::system::default_queue() };

  // Create buffers from a & b vectors
  buffer<float> A { std::begin(a), std::end(a) };
  buffer<float> B { std::begin(b), std::end(b) };

  {
    // A buffer of N float using the storage of c
    buffer<float> C { c, N };

    // Construct an OpenCL program from the source string
    auto program = boost::compute::program::create_with_source(R"(
      __kernel void vector_add(const __global float *a,
                               const __global float *b,
                               __global float *c,
                               int offset) {
        c[get_global_id(0)] = a[get_global_id(0)] + b[get_global_id(0)]
                              + offset;
      }
      )", boost::compute::system::default_context());

    // Build a kernel from the OpenCL kernel
    program.build();

    // Get the OpenCL kernel
    kernel k { boost::compute::kernel { program, "vector_add" } };

    // Launch the vector parallel addition
    q.submit([&](handler &cgh) {
        /* The host-device copies are managed transparently by these
           accessors: */
        cgh.set_args(A.get_access<access::mode::read>(cgh),
                     B.get_access<access::mode::read>(cgh),
                     C.get_access<access::mode::write>(cgh),
                     cl::sycl::cl_int { 42 } );
        cgh.parallel_for(N, k);
      }); //< End of our commands for this queue
  } //< Buffer C goes out of scope and copies back values to c

  REQUIRE(c[0] == 48);
  REQUIRE(c[1] == 50);
  REQUIRE(c[2] == 53);
}
