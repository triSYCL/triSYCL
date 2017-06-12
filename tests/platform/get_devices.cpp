/* RUN: %{execute}%s

   Test if platform::get_devices() work
*/

#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  platform p;

  vector_class<device> devices = p.get_devices();

#ifdef TRISYCL_OPENCL
  // Check if all the devices from boost compute are in the device vector
  for (const auto &d : boost::compute::system::devices()) {
    device s_d { d };
    BOOST_CHECK(std::find(devices.begin(), devices.end(),
                          s_d) != devices.end());
  }
  // Check if the host device is in the device vector
  device host_dev;
  BOOST_CHECK(std::find(devices.begin(), devices.end(),
                        host_dev) != devices.end());

#else
  // There should be only the host device
  BOOST_CHECK(devices.size() == 1);
  BOOST_CHECK(devices[0].is_host());
#endif

  return 0;
}
