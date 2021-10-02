/* RUN: %{execute}%s
*/
#include <boost/compute.hpp>

#include <catch2/catch_test_macros.hpp>

#include <CL/sycl.hpp>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

TEST_CASE("member functions of OpenCL queue", "[queue]") {
  // Start from an OpenCL queue
  auto oq = boost::compute::system::default_queue();
  // Construct a SYCL queue from it
  queue q { oq };

  check_all<queue>(q);

  // Check the OpenCL queue is still there behind the scene
  REQUIRE(oq == q.get());

  // Check that an OpenCL queue cannot be the host queue
  REQUIRE(!q.is_host());

  /// \todo Implement context
  // Check that the context behind is not host context
  //REQUIRE(!q.get_context().is_host());

  // Verify that the context behind the scene is the right one
  //REQUIRE(q.get_context().get() == oq.get_context().get());

  // Check that the device behind is not host device
  REQUIRE(!q.get_device().is_host());

  // Verify that the device behind the scene is the right one
  REQUIRE(q.get_device().get() == oq.get_device().get());
}
