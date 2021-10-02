/* RUN: %{execute}%s

   Test some vec<> access behaviour
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("access", "[vector]") {
  vec<int, 3> v = {1, 2, 3};
  REQUIRE(v.x() == 1);
  REQUIRE(v.y() == 2);
  REQUIRE(v.z() == 3);
  REQUIRE(v.get_count() == 3);
  REQUIRE(v.get_size() == 4 * sizeof(int));
  v.x() = 4;
  v.y() = 5;
  v.z() = 6;
  REQUIRE(v.x() == 4);
  REQUIRE(v.y() == 5);
  REQUIRE(v.z() == 6);

  vec<int, 1> v1 = { 1 };
  REQUIRE(v1 == 1);
}
