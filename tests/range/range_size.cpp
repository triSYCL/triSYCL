/* RUN: %{execute}%s

   Exercise range size-like methods
*/

#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("size", "[range]") {
  range<1> r1 { 8 };
  REQUIRE(r1.size() == 8);

  range<2> r2 { 3, 5 };
  REQUIRE(r2.size() == 3*5);

  range<3> r3 { 2, 7, 11 };
  REQUIRE(r3.size() == 2*7*11);

  range<2> r2_double { 7 };
  REQUIRE(r2_double.size() == 7*7);

  range<3> r3_triple { 11 };
  REQUIRE(r3_triple.size() == 11*11*11);
}
