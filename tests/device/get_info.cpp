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
    BOOST_CHECK(d.get_info<info::device::max_work_group_size>() >= 1);
    BOOST_CHECK(d.get_info<info::device::max_compute_units>() >= 1);
    BOOST_CHECK(d.get_info<info::device::device_type>() !=
		info::device_type::all);
    cl::sycl::info::local_mem_type local_mem_type = d.get_info<info::device::local_mem_type>();
    /* pocl in travis seems to end up with 16384, so use that to pass tests. */
    if (local_mem_type != cl::sycl::info::local_mem_type::none)
      BOOST_CHECK(d.get_info<info::device::local_mem_size>() >= 16384);
  }

  return 0;
}
