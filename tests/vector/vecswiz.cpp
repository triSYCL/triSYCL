/* RUN: %{execute}%s

   Test some vec<> simple swizzles behaviour
*/
#define SYCL_SIMPLE_SWIZZLES
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
	vec<int, 4> v = {1, 2, 3, 4};

	/* test an xyzw index */
	BOOST_CHECK(v.x() == 1);
	BOOST_CHECK(v.y() == 2);
	BOOST_CHECK(v.z() == 3);
	BOOST_CHECK(v.w() == 4);

	/* test a rgba index */
	BOOST_CHECK(v.r() == 1);
	BOOST_CHECK(v.g() == 2);
	BOOST_CHECK(v.b() == 3);
	BOOST_CHECK(v.a() == 4);

	/* test a xyzw swizzle */
	vec<int, 4> v2 = v.zyxx();
	BOOST_CHECK(v2.x() == 3);
	BOOST_CHECK(v2.y() == 2);
	BOOST_CHECK(v2.z() == 1);
	BOOST_CHECK(v2.w() == 1);

	/* test a rgba swizzle */
	vec<int, 4> v3 = v.rrbb();
	BOOST_CHECK(v3.r() == 1);
	BOOST_CHECK(v3.g() == 1);
	BOOST_CHECK(v3.b() == 3);
	BOOST_CHECK(v3.a() == 3);

	v.x() = 4;
	v.y() = 5;
	v.z() = 6;
	BOOST_CHECK(v.x() == 4);
	BOOST_CHECK(v.y() == 5);
	BOOST_CHECK(v.z() == 6);

	/* test a swizzle */
	v2 = v.swizzle<cl::sycl::elem::s1, cl::sycl::elem::s0, cl::sycl::elem::s2, cl::sycl::elem::s3>();
	BOOST_CHECK(v2.x() == 5);
	BOOST_CHECK(v2.y() == 4);
	BOOST_CHECK(v2.z() == 6);
	BOOST_CHECK(v2.w() == 4);

	return 0;
}
