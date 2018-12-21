/* RUN: %{execute}%s

   Test some vec<> simple swizzles behaviour
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

	/* test a lo */
	vec<int, 2> v2 = v.lo();
	BOOST_CHECK(v2.x() == 1);
	BOOST_CHECK(v2.y() == 2);

	vec<int, 4> v4 = {1, 2, 3, 4};

	/* test a hi on a 4 component */
	vec<int, 2> v3 = v4.hi();
	BOOST_CHECK(v3.x() == 3);
	BOOST_CHECK(v3.y() == 4);

	/* test a hi on a 3 component */
	vec<int, 2> v5 = v.hi();
	BOOST_CHECK(v5.x() == 3);

	/* test odd on 4 component */
	v3 = v4.odd();
	BOOST_CHECK(v3.x() == 2);
	BOOST_CHECK(v3.y() == 4);

	v3 = v4.even();
	BOOST_CHECK(v3.x() == 1);
	BOOST_CHECK(v3.y() == 3);
	return 0;
}
