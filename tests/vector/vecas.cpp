/* RUN: %{execute}%s

   Test some vec<> swizzle access behaviour
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("swizzle access", "[vector]") {
  vec<int, 4> v = {1, 2, 3, 4};

  REQUIRE(v.x() == 1);
  REQUIRE(v.y() == 2);
  REQUIRE(v.z() == 3);
  REQUIRE(v.w() == 4);

  auto v2 = v.as<cl::sycl::vec<unsigned int, 4>>();
  REQUIRE(v2.x() == 1);
  REQUIRE(v2.y() == 2);
  REQUIRE(v2.z() == 3);
  REQUIRE(v2.w() == 4);

  auto v3 = v.as<cl::sycl::vec<unsigned char, 16>>();
  REQUIRE(v3[0] == 1);
  REQUIRE(v3[4] == 2);
  REQUIRE(v3[8] == 3);
  REQUIRE(v3[12] == 4);

  vec<float, 4> vf = { 1.0, 2.0, 3.0, 4.0 };
  auto vi = vf.as<cl::sycl::vec<unsigned int, 4>>();
  REQUIRE(vi[0] == 0x3f800000);
  REQUIRE(vi[1] == 0x40000000);
  REQUIRE(vi[2] == 0x40400000);
  REQUIRE(vi[3] == 0x40800000);
}
