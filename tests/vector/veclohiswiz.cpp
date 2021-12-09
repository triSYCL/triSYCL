/* RUN: %{execute}%s

   Test some vec<> simple swizzles behaviour with .hi()), .odd()...
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("swizzles with .hi(), .odd()...construction and basic operations",
          "[vector]") {
  vec<int, 3> v = { 1, 2, 3 };

  REQUIRE(v.x() == 1);
  REQUIRE(v.y() == 2);
  REQUIRE(v.z() == 3);

  /* test a lo */
  vec<int, 2> v2 = v.lo();
  REQUIRE(v2.x() == 1);
  REQUIRE(v2.y() == 2);

  vec<int, 4> v4 = { 1, 2, 3, 4 };

  /* test a hi on a 4 component */
  vec<int, 2> v3 = v4.hi();
  REQUIRE(v3.x() == 3);
  REQUIRE(v3.y() == 4);

  /* test a hi on a 3 component */
  vec<int, 2> v5 = v.hi();
  REQUIRE(v5.x() == 3);

  /* test odd on 4 component */
  v3 = v4.odd();
  REQUIRE(v3.x() == 2);
  REQUIRE(v3.y() == 4);

  v3 = v4.even();
  REQUIRE(v3.x() == 1);
  REQUIRE(v3.y() == 3);
}
