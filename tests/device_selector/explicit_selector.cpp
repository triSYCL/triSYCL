/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <algorithm>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

template <typename DeviceSelector>
void test_device(info::device_type type)
/* Since the test platform may not have all the requested device
   kinds, protect with a try/catch */
try {
  // Get the best device according to the device_selector
  device d { DeviceSelector {} };
  // Verify the device is in the list of devices of that kind
  auto ds = device::get_devices(type);
  auto f = std::find(ds.cbegin(), ds.cend(), d);
  BOOST_CHECK(f != ds.cend());
}
/// \todo put the final exception class here
catch (std::domain_error &e) {
}


int test_main(int argc, char *argv[]) {
  test_device<host_selector>(info::device_type::host);
  test_device<cpu_selector>(info::device_type::cpu);
  test_device<gpu_selector>(info::device_type::gpu);
  test_device<default_selector>(info::device_type::automatic);
  return 0;
}
