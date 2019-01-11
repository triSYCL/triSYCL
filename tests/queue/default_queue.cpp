/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>

#include <boost/test/minimal.hpp>

#include "basic_object_checks.hpp"
#include "check_throwing_get.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  check_all<queue>(queue{});
  queue q;

  // Check that this is the host queue by default
  BOOST_CHECK(q.is_host());

  // Check that the context behind is the host context
  BOOST_CHECK(q.get_context().is_host());

  // Check that the device behind is the host device
  BOOST_CHECK(q.get_device().is_host());

  // Verify that get() throws since there is no OpenCL behind the curtain
  check_throwing_get(q);

  return 0;
}
