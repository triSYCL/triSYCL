/* RUN: %{execute}%s

   Test some vec<> behaviour
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
	vec<int, 3> v = {1, 2, 3};
	BOOST_CHECK(v.x() == 1);
	BOOST_CHECK(v.y() == 2);
	BOOST_CHECK(v.z() == 3);
	v.x() = 4;
	v.y() = 5;
	v.z() = 6;
	BOOST_CHECK(v.x() == 4);
	BOOST_CHECK(v.y() == 5);
	BOOST_CHECK(v.z() == 6);
  return 0;
}
