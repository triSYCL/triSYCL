/* RUN: %{execute}%s

   Check single_task() execution of a kernel with OpenCL
   interoperability mode.
*/
#include <iostream>
#include <iterator>
#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

using namespace cl::sycl;

constexpr size_t N = 3;
using Vector = float[N];

int test_main(int argc, char *argv[]) {
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
                               int offset,
                               int size) {
        for (int i = 0; i < size; ++i)
          c[i] = a[i] + b[i] + offset;
      }
      )", boost::compute::system::default_context());

    // Build a kernel from the OpenCL kernel
    program.build();

    // Get the OpenCL kernel
    kernel k { boost::compute::kernel { program, "vector_add" } };

    // Launch a single instance of the kernel doing the addition
    q.submit([&](handler &cgh) {
        /* The host-device copies are managed transparently by these
           accessors: */
        cgh.set_args(A.get_access<access::mode::read>(cgh),
                     B.get_access<access::mode::read>(cgh),
                     C.get_access<access::mode::write>(cgh),
                     /* TODO: use cl::sycl::cl_int { 42 } for portability
                        when it is implemented */
                     42,
                     int { N });
        cgh.single_task(k);
      }); //< End of our commands for this queue
  } //< Buffer C goes out of scope and copies back values to c

  // Verify the computation done by the kernel
  for (std::size_t i = 0; i < N; ++i)
    BOOST_CHECK(c[i] == a[i] + b[i] + 42);

  return 0;
}
