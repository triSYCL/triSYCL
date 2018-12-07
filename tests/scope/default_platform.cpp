/* RUN: %{execute}%s
 */
#include <CL/sycl.hpp>
#include <iostream>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;
namespace bh = boost::hana;

int test_main(int argc, char *argv[]) {

  auto p = extension::ce::platform::get_platforms();

  std::cout << (bh::make_pair(info::platform::vendor,
                              "The pocl project")
                == bh::make_pair(info::platform::profile,
                                "FULL_PROFILE"));

  auto f = extension::ce::platform::find();
  boost::hana::for_each(p, [](auto plat) {
      std::cout << plat.get_name() << std::endl;
        });
  return 0;
}
