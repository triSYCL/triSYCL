/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("OpenCL selector", "[device selector]") {
  // Get all the devices from the given device_type
  auto opencl_devices = device::get_devices(info::device_type::opencl);
  auto all_devices = device::get_devices();
  /* Since the OpenCL devices are not the host one, there should be one
     less device than all devices */
  std::cout << opencl_devices.size() << std::endl;
  std::cout << all_devices.size() << std::endl;
  REQUIRE(opencl_devices.size() == all_devices.size() - 1);
  // Check that each OpenCL device is not the host
  for (auto const &d : opencl_devices)
    REQUIRE(!d.is_host());
}
