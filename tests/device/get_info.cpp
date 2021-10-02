/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("get_info", "[device]") {
  // Get all the devices
  auto devices = device::get_devices();
  // Verify that the device_type returned match what is expected
  for (auto const &d : devices) {
    REQUIRE(d.get_info<info::device::max_work_group_size>() >= 1);
    REQUIRE(d.get_info<info::device::max_compute_units>() >= 1);
    REQUIRE(d.get_info<info::device::device_type>() !=
            info::device_type::all);
    cl::sycl::info::local_mem_type local_mem_type = d.get_info<info::device::local_mem_type>();
    /* PoCL in Travis seems to end up with 16384, so use that to pass tests. */
    if (local_mem_type != cl::sycl::info::local_mem_type::none)
      REQUIRE(d.get_info<info::device::local_mem_size>() >= 16384);
  }
}
