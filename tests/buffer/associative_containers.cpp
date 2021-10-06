/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>

#include <iostream>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

TEST_CASE("basic behaviors", "[buffer]") {
  check_all<buffer<char, 3>>();
}
