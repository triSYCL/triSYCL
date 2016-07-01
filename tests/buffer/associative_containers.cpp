/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <boost/test/minimal.hpp>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  check_all<buffer<char, 3>>();

  return 0;
}
