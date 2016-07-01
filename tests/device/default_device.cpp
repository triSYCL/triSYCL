/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <set>

#include <boost/test/minimal.hpp>

#include "basic_object_checks.hpp"
#include "check_throwing_get.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Create a default device which is the host device
  device d;

  // Check it is really the host device
  BOOST_CHECK(d.is_host());
  BOOST_CHECK(!d.is_cpu());
  BOOST_CHECK(!d.is_gpu());
  BOOST_CHECK(!d.is_accelerator());

  // Basic checks on associative containers
  check_all(d);

  // Check that the host device appears in the device list exactly once
  auto l = device::get_devices();
  BOOST_CHECK(std::count(l.begin(), l.end(), d) == 1);

  // Verify that get() throws since there is no OpenCL behind the curtain
  check_throwing_get(d);

  return 0;
}
