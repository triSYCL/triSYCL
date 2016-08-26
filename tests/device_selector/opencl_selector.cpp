/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Get all the devices from the given device_type
  auto opencl_devices = device::get_devices(info::device_type::opencl);
  auto all_devices = device::get_devices();
  /* Since the OpenCL devices are not the host one, there should be one
     less device than all devices */
  std::cout << opencl_devices.size() << std::endl;
  std::cout << all_devices.size() << std::endl;
  BOOST_CHECK(opencl_devices.size() == all_devices.size() - 1);
  // Check that each OpenCL device is not the host
  for (auto const &d : opencl_devices)
    BOOST_CHECK(!d.is_host());

  return 0;
}
