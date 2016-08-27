/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>
#include <iostream>

// The #ifdef is to remember this test requires OpenCL to run...
#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
#include <boost/test/minimal.hpp>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Create an OpenCL device
  check_all<device>(boost::compute::system::devices()[0]);

  device d { boost::compute::system::devices()[0] };
  // Check that it cannot be the host device
  BOOST_CHECK(!d.is_host());
  // Check it is one of this type
  BOOST_CHECK(d.is_cpu() + d.is_gpu() + d.is_accelerator() == 1);

  // Verify that the cache works when asking for same device
  device d3 = boost::compute::system::devices()[0];
  // Check the host device is actually a singleton
  BOOST_CHECK(d == d3);

  // Verify the construction from cl_device_id
  auto all_devices = device::get_devices();
  std::set<device> devices = { all_devices.begin(), all_devices.end() };
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
