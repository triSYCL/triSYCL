/* RUN: %{execute}%s

   Exercise accessor size-like methods

   Related to https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564
   and https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
*/

#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>
#include <iostream>

using namespace cl::sycl;

#define CHECK_RANGE_ACCESSOR(r, a)                                \
  BOOST_CHECK(r  == a.get_range());                               \
  BOOST_CHECK(r.size() == a.get_count());                         \
  BOOST_CHECK(a.get_size()                                        \
              == a.get_count()*sizeof(decltype(a)::value_type));


int test_main(int argc, char *argv[]) {
  range<1> r1 { 8 };
  buffer<double> b1 { r1 };
  CHECK_RANGE_ACCESSOR(r1, (b1.get_access<access::mode::read_write>()));

  range<2> r2 { 3, 5 };
  buffer<short, 2> b2 { r2 };
  CHECK_RANGE_ACCESSOR(r2, (b2.get_access<access::mode::read_write>()));

  range<3> r3 { 2, 7, 11 };
  buffer<short, 3> b3 { r3 };
  CHECK_RANGE_ACCESSOR(r3, (b3.get_access<access::mode::read_write>()));

  return 0;
}
