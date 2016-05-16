/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <set>

// The #ifdef is to remember this test requires OpenCL to run...
#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
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

  // Verify that the cache works when asking for same device
  device d3 = boost::compute::system::devices()[0];
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

  // Verify the construction from cl_device_id
  auto all_devices = device::get_devices();
  devices = { all_devices.begin(), all_devices.end() };
  // Remove the host device from the list
  devices.erase(device {});

  // Reconstruct the OpenCL devices from their cl_device_id:
  std::vector<device> od;
  for (const auto &bd : boost::compute::system::devices())
    od.emplace_back(bd.id());

  // Compare the elements from SYCL and from Boost.Compute
  BOOST_CHECK(std::is_permutation(devices.begin(), devices.end(),
                                  od.begin(), od.end()));

  return 0;
}
