/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("type of devices", "[device]") {
  // Get all the devices
  auto devices = device::get_devices();
  // Verify that the device_type returned match what is expected
  for (auto const &d : devices) {
    REQUIRE((d.type() == info::device_type::host) == d.is_host());
    REQUIRE((d.type() == info::device_type::cpu) == d.is_cpu());
    REQUIRE((d.type() == info::device_type::gpu) == d.is_gpu());
    REQUIRE((d.type() == info::device_type::accelerator)
            == d.is_accelerator());
  }
}
