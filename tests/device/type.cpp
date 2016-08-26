/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Get all the devices
  auto devices = device::get_devices();
  // Verify that the device_type returned match what is expected
  for (auto const &d : devices) {
    BOOST_CHECK((d.type() == info::device_type::host) == d.is_host());
    BOOST_CHECK((d.type() == info::device_type::cpu) == d.is_cpu());
    BOOST_CHECK((d.type() == info::device_type::gpu) == d.is_gpu());
    BOOST_CHECK((d.type() == info::device_type::accelerator)
                == d.is_accelerator());
  }

  return 0;
}
