/* RUN: %{execute}%s

   Test some vec<> behaviour about structured binding
*/
#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>

int test_main(int argc, char *argv[]) {
  {
    cl::sycl::vec<int, 1> v1i = { 12 };
    const auto [ x ] = v1i;
    // Note that it is just a naming coincidence to have x in both lhs and rhs
    BOOST_CHECK(x == v1i.x());
  }
  {
    cl::sycl::vec<int, 2> v2i = { 1, 2 };
    const auto [ x, y ] = v2i;
    BOOST_CHECK(x == v2i.x());
    BOOST_CHECK(y == v2i.y());
  }
  {
    cl::sycl::vec<int, 3> v3i = { 1, 2, 3 };
    const auto [ x, y, z ] = v3i;
    BOOST_CHECK(x == v3i.x());
    BOOST_CHECK(y == v3i.y());
    BOOST_CHECK(z == v3i.z());
  }
  {
    cl::sycl::vec<int, 4> v4i = { 1, 2, 3, 4 };
    const auto [ r, g, b, a ] = v4i;
    BOOST_CHECK(r == v4i.r());
    BOOST_CHECK(g == v4i.g());
    BOOST_CHECK(b == v4i.b());
    BOOST_CHECK(a == v4i.a());
  }
  return 0;
}
