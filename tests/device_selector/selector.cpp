/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("existing selectors", "[device selector]") {
  // For all device_type
  for (auto const &t : { info::device_type::host,
                         info::device_type::cpu,
                         info::device_type::gpu,
                         info::device_type::accelerator }) {
    // Get all the devices from the given device_type
    auto devices = device::get_devices(t);
    // Check that each returned device has the expected type
    for (auto const &d : devices)
      REQUIRE(d.type() == t);
  }
}
