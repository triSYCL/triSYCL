/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

template <typename DeviceSelector>
void test_device(info::device_type type)
/* Since the test platform may not have all the requested device
   kinds, protect with a try/catch */
try {
  // Get the best device according to the device_selector
  device d { DeviceSelector {} };
  // Verify the device is in the list of devices of that kind
  auto ds = device::get_devices(type);
  auto f = std::find(ds.cbegin(), ds.cend(), d);
  REQUIRE(f != ds.cend());
}
/// \todo put the final exception class here
catch (std::domain_error &e) {
}


TEST_CASE("explicit selector", "[device selector]") {
  test_device<host_selector>(info::device_type::host);
  test_device<cpu_selector>(info::device_type::cpu);
  test_device<gpu_selector>(info::device_type::gpu);
  test_device<default_selector>(info::device_type::automatic);
}
