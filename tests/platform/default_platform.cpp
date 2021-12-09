/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: is_host() = 1
   CHECK-NEXT: get_info<info::platform::profile>() = FULL_PROFILE
   CHECK-NEXT: get_info<info::platform:::version>() = 2.2
   CHECK-NEXT: get_info<info::platform::name>() = triSYCL host platform
   CHECK-NEXT: get_info<info::platform::vendor>() = triSYCL Open Source project
   CHECK-NEXT: get_info<info::platform::extensions>() = Xilinx_blocking_pipes
   CHECK-NEXT: is_host() = 1
   CHECK-NEXT: get_info<info::platform::profile>() = FULL_PROFILE
   CHECK-NEXT: get_info<info::platform:::version>() = 2.2
   CHECK-NEXT: get_info<info::platform::name>() = triSYCL host platform
   CHECK-NEXT: get_info<info::platform::vendor>() = triSYCL Open Source project
   CHECK-NEXT: get_info<info::platform::extensions>() = Xilinx_blocking_pipes
   CHECK: is_host() = 1
   CHECK-NEXT: get_info<info::platform::profile>() = FULL_PROFILE
   CHECK-NEXT: get_info<info::platform:::version>() = 2.2
   CHECK-NEXT: get_info<info::platform::name>() = triSYCL host platform
   CHECK-NEXT: get_info<info::platform::vendor>() = triSYCL Open Source project
   CHECK-NEXT: get_info<info::platform::extensions>() = Xilinx_blocking_pipes
*/
#include <CL/sycl.hpp>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "basic_object_checks.hpp"
#include "check_throwing_get.hpp"
#include "display_platform.hpp"

using namespace cl::sycl;

TEST_CASE("member functions with default platform", "[platform]") {
  check_all<platform>();
  platform p;
  display(p);

  // Check that it is the host platform by default
  REQUIRE(p.is_host());

  platform p2 { p };

  display(p2);

//  std::cout << "(p == p2) should be printed as 1: " << (p == p2) << std::endl
//            << std::endl;

  for (const auto &p : platform::get_platforms()) {
    std::cout << "Platform " << &p << ':' << std::endl;
    display(p);
  }

  // Verify that get() throws since there is no OpenCL behind the curtain
  check_throwing_get(p);
}
