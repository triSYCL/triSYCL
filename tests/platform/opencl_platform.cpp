/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>

// The #ifdef is to remember this test requires OpenCL to run...
#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
#include <boost/test/minimal.hpp>

#include "basic_object_checks.hpp"
#include "display_platform.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  check_all<platform>(boost::compute::system::platforms()[0]);
  platform p { boost::compute::system::platforms()[0] };
  display(p);

  // Check that it cannot be the host platform by default
  BOOST_CHECK(!p.is_host());

  // Verify that the cache works when asking for same platform
  platform p3 = boost::compute::system::platforms()[0];
  // Check the host platform is actually a singleton
  BOOST_CHECK(p == p3);

  // Verify the construction from cl_platform_id
  auto all_platforms = platform::get_platforms();
  std::set<platform> platforms = { all_platforms.begin(),
                                   all_platforms.end() };
  // Remove the host platform from the list
  platforms.erase(platform {});

  // Reconstruct the OpenCL platforms from their cl_platform_id:
  std::vector<platform> op;
  for (const auto &bp : boost::compute::system::platforms())
    op.emplace_back(bp.id());

  // Compare the elements from SYCL and from Boost.Compute
  BOOST_CHECK(std::is_permutation(platforms.begin(), platforms.end(),
                                  op.begin(), op.end()));

  return 0;
}
