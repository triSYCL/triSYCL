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

#include <boost/test/minimal.hpp>

#include "associative_container_checks.hpp"

using namespace cl::sycl;


void display(const platform &p) {
  std::cout << "is_host() = " << p.is_host() << std::endl;
  std::cout << "get_info<info::platform::profile>() = "
            << p.get_info<info::platform::profile>() << std::endl;
  std::cout << "get_info<info::platform:::version>() = "
            << p.get_info<info::platform::version>() << std::endl;
  std::cout << "get_info<info::platform::name>() = "
            << p.get_info<info::platform::name>() << std::endl;
  std::cout << "get_info<info::platform::vendor>() = "
            << p.get_info<info::platform::vendor>() << std::endl;
  std::cout << "get_info<info::platform::extensions>() = "
            << p.get_info<info::platform::extensions>() << std::endl;
}


int test_main(int argc, char *argv[]) {
  associative_container_checks<device>({});
  platform p;
  display(p);

  // Check that it is the host platform by default
  BOOST_CHECK(p.is_host());

  platform p2 { p };

  display(p2);

//  std::cout << "(p == p2) should be printed as 1: " << (p == p2) << std::endl
//            << std::endl;

  for (const auto &p : platform::get_platforms()) {
    std::cout << "Platform " << &p << ':' << std::endl;
    display(p);
  }

  return 0;
}
