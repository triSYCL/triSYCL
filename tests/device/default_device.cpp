/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <iostream>
#include <unordered_set>
#include <set>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Create a default device which is the host device
  device d;

  // Check it is really the host device
  BOOST_CHECK(d.is_host());
  BOOST_CHECK(!d.is_cpu());
  BOOST_CHECK(!d.is_gpu());
  BOOST_CHECK(!d.is_accelerator());

  // Verify the copy operation
  device d2 { d };
  BOOST_CHECK(d == d2);

  device d3;
  // Check the host device is actually a singleton
  //BOOST_CHECK(d == d3);

  // Check device is comparable so it can be put in associative containers
  std::set<device> devices;
  devices.insert(d);
  devices.insert(d2);
  devices.insert(d3);

  std::cout << devices.size() << std::endl;
  // Check the host device is actually a singleton
  //BOOST_CHECK(devices.size() == 1);

  // Check device is comparable so it can be put in associative containers
  std::unordered_set<device> ud;
  ud.insert(d);
  ud.insert(d2);
  ud.insert(d3);

  std::cout << ud.size() << std::endl;
  // Check the host device is actually a singleton
  //BOOST_CHECK(ud.size() == 1);

  return 0;
}
