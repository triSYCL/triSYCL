/* RUN: %{execute}%s

   Test some vec<> simple swizzles behaviour
*/
#define SYCL_SIMPLE_SWIZZLES
#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace cl::sycl;

TEST_CASE("swizzles", "[vector]") {
  vec<int, 4> v = { 1, 2, 3, 4 };

  /* test an xyzw index */
  REQUIRE(v.x() == 1);
  REQUIRE(v.y() == 2);
  REQUIRE(v.z() == 3);
  REQUIRE(v.w() == 4);

  /* test a rgba index */
  REQUIRE(v.r() == 1);
  REQUIRE(v.g() == 2);
  REQUIRE(v.b() == 3);
  REQUIRE(v.a() == 4);

  /* test a xyzw swizzle */
  vec<int, 4> v2 = v.zyxx();
  REQUIRE(v2.x() == 3);
  REQUIRE(v2.y() == 2);
  REQUIRE(v2.z() == 1);
  REQUIRE(v2.w() == 1);

  /* test a rgba swizzle */
  vec<int, 4> v3 = v.rrbb();
  REQUIRE(v3.r() == 1);
  REQUIRE(v3.g() == 1);
  REQUIRE(v3.b() == 3);
  REQUIRE(v3.a() == 3);

  v.x() = 4;
  v.y() = 5;
  v.z() = 6;
  REQUIRE(v.x() == 4);
  REQUIRE(v.y() == 5);
  REQUIRE(v.z() == 6);

  /* test a swizzle */
  v2 = v.swizzle<cl::sycl::elem::s1, cl::sycl::elem::s0, cl::sycl::elem::s2,
                 cl::sycl::elem::s3>();
  REQUIRE(v2.x() == 5);
  REQUIRE(v2.y() == 4);
  REQUIRE(v2.z() == 6);
  REQUIRE(v2.w() == 4);
}
