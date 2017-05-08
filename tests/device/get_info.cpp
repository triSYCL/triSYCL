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
    BOOST_CHECK(d.get_info<info::device::max_work_group_size>() == 8);
    BOOST_CHECK(d.get_info<info::device::max_compute_units>() == 8);
    BOOST_CHECK(d.get_info<info::device::device_type>() ==
        info::device_type::cpu);
    BOOST_CHECK(d.get_info<info::device::local_mem_size>() == 32000);
  }

  return 0;
}
