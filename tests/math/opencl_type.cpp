/* RUN: %{execute}%s | %{filecheck} %s
  CHECK: x: 0, y: 1, z: 2
*/

/* Test opencl_type behaviour
*/

#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>


int test_main(int argc, char *argv[]) {
  cl::sycl::cl_float3 vec { 0, 1, 2 };
  std::cout << "x: "    << vec.x()
            << ", y: "  << vec.y()
            << ", z: "  << vec.z() << std::endl;
  return 0;
}
