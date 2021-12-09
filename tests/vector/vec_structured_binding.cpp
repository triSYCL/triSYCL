/* RUN: %{execute}%s

   Test some vec<> behaviour about structured binding
*/
#include <CL/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("structured binding", "[vector]") {
  {
    cl::sycl::vec<int, 1> v1i = { 12 };
    const auto [ x ] = v1i;
    // Note that it is just a naming coincidence to have x in both lhs and rhs
    REQUIRE(x == v1i.x());
  }
  {
    cl::sycl::vec<int, 2> v2i = { 1, 2 };
    const auto [ x, y ] = v2i;
    REQUIRE(x == v2i.x());
    REQUIRE(y == v2i.y());
  }
  {
    cl::sycl::vec<int, 3> v3i = { 1, 2, 3 };
    const auto [ x, y, z ] = v3i;
    REQUIRE(x == v3i.x());
    REQUIRE(y == v3i.y());
    REQUIRE(z == v3i.z());
  }
  {
    cl::sycl::vec<int, 4> v4i = { 1, 2, 3, 4 };
    const auto [ r, g, b, a ] = v4i;
    REQUIRE(r == v4i.r());
    REQUIRE(g == v4i.g());
    REQUIRE(b == v4i.b());
    REQUIRE(a == v4i.a());
  }
}
