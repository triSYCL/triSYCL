/* RUN: %{execute}%s
*/
#include <iostream>
#include <boost/compute.hpp>

#include <catch2/catch_test_macros.hpp>

#include <CL/sycl.hpp>

#include "basic_object_checks.hpp"

using namespace cl::sycl;

TEST_CASE("OpenCL kernel", "[kernel]") {
  // Construct an OpenCL program from the source string
  auto program = boost::compute::program::create_with_source(R"(
    __kernel void empty() {
    }
    )", boost::compute::system::default_context());
  // Build a kernel from the OpenCL kernel
  program.build();
  /* Get the OpenCL kernel as its own variable for extended lifetime
     so we do not have to have a clRetainKernel */
  auto bk = boost::compute::kernel { program, "empty" };
  kernel k { bk.get() };

  // Verify that the cache works when asking for same kernel
  kernel k2 { bk.get() };
  REQUIRE(k == k2);

  /* Do not use the default assignment check check_assignment() since
     there is no default constructor. Use an explicit check */
  k2 = k;
  REQUIRE(k == k2);

  // The other usual checks
  check_copy(k);
  check_associative_ordered_container(k);
  check_associative_unordered_container(k);
}
