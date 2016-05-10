/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  // Create a default device which is the host device
  device d;
  BOOST_CHECK(d.is_host());
  BOOST_CHECK(!d.is_cpu());
  BOOST_CHECK(!d.is_gpu());
  BOOST_CHECK(!d.is_accelerator());
  device d2 { d };

  //BOOST_CHECK(d == d2);
  return 0;
}
