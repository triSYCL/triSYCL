/* RUN: %{execute}%s

   Exercise buffer size-like member functions
*/

#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

#define CHECK_RANGE_BUFFER(r, b)                                  \
  REQUIRE(r  == b.get_range());                                   \
  REQUIRE(r.size() == b.get_count());                             \
  REQUIRE(b.get_size()                                            \
          == b.get_count()*sizeof(decltype(b)::value_type));


TEST_CASE("size-like member functions", "[buffer]") {
  range<1> r1 { 8 };
  buffer<double> b1 { r1 };
  CHECK_RANGE_BUFFER(r1, b1);

  range<2> r2 { 3, 5 };
  buffer<short, 2> b2 { r2 };
  CHECK_RANGE_BUFFER(r2, b2);

  range<3> r3 { 2, 7, 11 };
  buffer<short, 3> b3 { r3 };
  CHECK_RANGE_BUFFER(r3, b3);
}
