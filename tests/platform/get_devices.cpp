/* RUN: %{execute}%s

   Test if platform::get_devices() work
*/

#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("get_device", "[platform]") {

#ifdef TRISYCL_OPENCL
  // We check that \c get_device in triSYCL returns the same devices
  // that \c get_device in Boost Compute
  for (const auto &p : boost::compute::system::platforms()) {

    platform s_p { p };
    std::vector<device> devices = s_p.get_devices();

    for(const auto &d : p.devices()) {
      device s_d { d };
      REQUIRE(std::find(devices.begin(), devices.end(),
                        s_d) != devices.end());
    }
  }
#endif

  // We test get_devices for the host platform
  platform p;
  vector_class<device> devices = p.get_devices();

  // There should be only the host device
  REQUIRE(devices.size() == 1);
  REQUIRE(devices[0].is_host());
}
