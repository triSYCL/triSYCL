/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <iostream>

#include <boost/test/minimal.hpp>

#include "associative_container_checks.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  associative_container_checks<queue>({});
  queue q;

  // Check that this is the host queue by default
  BOOST_CHECK(q.is_host());

  return 0;
}
