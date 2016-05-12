/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <boost/test/minimal.hpp>

#include "associative_container_checks.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  associative_container_checks<buffer<char, 3>>();

  return 0;
}
