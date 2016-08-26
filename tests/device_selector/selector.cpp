/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // For all device_type
  for (auto const &t : { info::device_type::host,
                         info::device_type::cpu,
                         info::device_type::gpu,
                         info::device_type::accelerator }) {
    // Get all the devices from the given device_type
    auto devices = device::get_devices(t);
    // Check that each returned device has the expected type
    for (auto const &d : devices)
      BOOST_CHECK(d.type() == t);
  }

  return 0;
}
