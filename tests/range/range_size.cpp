/* RUN: %{execute}%s

   Exercise range size-like methods
*/

#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int /*argc*/, char*[] /*argv*/) {
  range<1> r1 { 8 };
  BOOST_CHECK(r1.size() == 1);
  BOOST_CHECK(r1.get_count() == 8);

  range<2> r2 { 3, 5 };
  BOOST_CHECK(r2.size() == 2);
  BOOST_CHECK(r2.get_count() == 3*5);

  range<3> r3 { 2, 7, 11 };
  BOOST_CHECK(r3.size() == 3);
  BOOST_CHECK(r3.get_count() == 2*7*11);

  range<2> r2_double { 7 };
  BOOST_CHECK(r2_double.size() == 2);
  BOOST_CHECK(r2_double.get_count() == 7*7);

  range<3> r3_triple { 11 };
  BOOST_CHECK(r3_triple.size() == 3);
  BOOST_CHECK(r3_triple.get_count() == 11*11*11);

  return 0;
}
