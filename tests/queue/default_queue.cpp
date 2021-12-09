/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "basic_object_checks.hpp"
#include "check_throwing_get.hpp"

using namespace cl::sycl;

TEST_CASE("member functions of default queue", "[queue]") {
  check_all<queue>(queue{});
  queue q;

  // Check that this is the host queue by default
  REQUIRE(q.is_host());

  // Check that the context behind is the host context
  REQUIRE(q.get_context().is_host());

  // Check that the device behind is the host device
  REQUIRE(q.get_device().is_host());

  // Verify that get() throws since there is no OpenCL behind the curtain
  check_throwing_get(q);
}
