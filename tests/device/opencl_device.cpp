/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <set>

#include <boost/compute.hpp>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Create an OpenCL device
  device d { boost::compute::system::devices()[0] };

  // Check that it cannot be the host device
  BOOST_CHECK(!d.is_host());
  // Check it is one of this type
  BOOST_CHECK(d.is_cpu() + d.is_gpu() + d.is_accelerator() == 1);

  // Verify the copy operation
  device d2 { d };
  BOOST_CHECK(d == d2);

  device d3;
  // Check the host device is actually a singleton
  BOOST_CHECK(d == d3);

  // Check device is comparable so it can be put in associative containers
  std::set<device> devices;
  devices.insert(d);
  devices.insert(d2);
  devices.insert(d3);

  // Check the host device is actually a singleton even in an ordered set
  BOOST_CHECK(devices.size() == 1);

  // Check device is comparable so it can be put in associative containers
  std::unordered_set<device> ud;
  ud.insert(d);
  ud.insert(d2);
  ud.insert(d3);

  // Check the host device is actually a singleton even in an unordered set
  BOOST_CHECK(ud.size() == 1);

  // Check that the host device appears in the device list exactly once
  auto l = device::get_devices();
  BOOST_CHECK(std::count(l.begin(), l.end(), d) == 1);

  bool exception_seen = false;
  try {
    // Try to get the non existent CL device
    d.get();
  }
  catch (non_cl_error e) {
    exception_seen = true;
  }
  // Check the error was well managed
  BOOST_CHECK(exception_seen);

  return 0;
}
