/* RUN: %{execute}%s
 */
#include <CL/sycl.hpp>
#include <iostream>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {

  auto p = extension::ce::platform::get_platforms();

  boost::hana::for_each(p, [](auto plat) {
      std::cout << plat.get_name() << std::endl;
        });
  return 0;
}
