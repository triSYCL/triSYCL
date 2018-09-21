/* RUN: %{execute}%s

   Exercise buffer size-like methods
*/

#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>
#include <iostream>

using namespace cl::sycl;

#define CHECK_RANGE_BUFFER(r, b)                                  \
  BOOST_CHECK(r  == b.get_range());                               \
  BOOST_CHECK(r.size() == b.get_count());                         \
  BOOST_CHECK(b.get_size()                                        \
              == b.get_count()*sizeof(decltype(b)::value_type));


int test_main(int argc, char *argv[]) {
  range<1> r1 { 8 };
  buffer<double> b1 { r1 };
  CHECK_RANGE_BUFFER(r1, b1);

  range<2> r2 { 3, 5 };
  buffer<short, 2> b2 { r2 };
  CHECK_RANGE_BUFFER(r2, b2);

  range<3> r3 { 2, 7, 11 };
  buffer<short, 3> b3 { r3 };
  CHECK_RANGE_BUFFER(r3, b3);

  return 0;
}
