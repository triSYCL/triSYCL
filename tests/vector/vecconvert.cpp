/* RUN: %{execute}%s

   Test some vec<> array access
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("array access", "[vector]") {
  vec<float, 8> v = {1.0f, 2.5f, 3.2f, 4.8f, 0.0f, -1.0f, -2.5f, -4.8f};

  REQUIRE(v[0] == 1.0f);
  REQUIRE(v[1] == 2.5f);
  REQUIRE(v[2] == 3.2f);
  REQUIRE(v[3] == 4.8f);
  REQUIRE(v[4] == 0.0f);
  REQUIRE(v[5] == -1.0f);
  REQUIRE(v[6] == -2.5f);
  REQUIRE(v[7] == -4.8f);

  auto v2 = v.convert<int, cl::sycl::rounding_mode::automatic>();
  REQUIRE(v2[0] == 1);
  REQUIRE(v2[1] == 2);
  REQUIRE(v2[2] == 3);
  REQUIRE(v2[3] == 4);
  REQUIRE(v2[4] == 0);
  REQUIRE(v2[5] == -1);
  REQUIRE(v2[6] == -2);
  REQUIRE(v2[7] == -4);
}
