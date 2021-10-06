/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <set>

#include <catch2/catch_test_macros.hpp>

#include "basic_object_checks.hpp"
#include "check_throwing_get.hpp"

using namespace cl::sycl;

TEST_CASE("default device", "[device]") {
  // Create a default device which is the host device
  device d;

  // Check it is really the host device
  REQUIRE(d.is_host());
  REQUIRE(!d.is_cpu());
  REQUIRE(!d.is_gpu());
  REQUIRE(!d.is_accelerator());

  // Basic checks on associative containers
  check_all(d);

  // Check that the host device appears in the device list exactly once
  auto l = device::get_devices();
  REQUIRE(std::count(l.begin(), l.end(), d) == 1);

  // Verify that get() throws since there is no OpenCL behind the curtain
  check_throwing_get(d);
}
