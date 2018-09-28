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
	v++;
	BOOST_CHECK(v.x() == 2);
	BOOST_CHECK(v.y() == 3);
	BOOST_CHECK(v.z() == 4);
	++v;
	BOOST_CHECK(v.x() == 3);
	BOOST_CHECK(v.y() == 4);
	BOOST_CHECK(v.z() == 5);
	v--;
	BOOST_CHECK(v.x() == 2);
	BOOST_CHECK(v.y() == 3);
	BOOST_CHECK(v.z() == 4);
	--v;
	BOOST_CHECK(v.x() == 1);
	BOOST_CHECK(v.y() == 2);
	BOOST_CHECK(v.z() == 3);

	vec<int, 3> v2 = { 4, 4, 4 };
	v |= v2;
	BOOST_CHECK(v.x() == 5);
	BOOST_CHECK(v.y() == 6);
	BOOST_CHECK(v.z() == 7);

	v &= v2;
	BOOST_CHECK(v.x() == 4);
	BOOST_CHECK(v.y() == 4);
	BOOST_CHECK(v.z() == 4);

	vec<int, 3> v3 = { 4, 5, 6 };
	v ^= v3;
	BOOST_CHECK(v.x() == 0);
	BOOST_CHECK(v.y() == 1);
	BOOST_CHECK(v.z() == 2);

	v = {1, 2, 3};
	vec<int, 3> vres = v | v2;
	BOOST_CHECK(vres.x() == 5);
	BOOST_CHECK(vres.y() == 6);
	BOOST_CHECK(vres.z() == 7);

	vres = v & v2;
	BOOST_CHECK(vres.x() == 0);
	BOOST_CHECK(vres.y() == 0);
	BOOST_CHECK(vres.z() == 0);

	vres = v3 ^ v2;
	BOOST_CHECK(vres.x() == 0);
	BOOST_CHECK(vres.y() == 1);
	BOOST_CHECK(vres.z() == 2);

	v |= 4;
	BOOST_CHECK(v.x() == 5);
	BOOST_CHECK(v.y() == 6);
	BOOST_CHECK(v.z() == 7);

	v &= 4;
	BOOST_CHECK(v.x() == 4);
	BOOST_CHECK(v.y() == 4);
	BOOST_CHECK(v.z() == 4);

	v = {1, 2, 3};
	vec<int, 3> v4 = ~v;
	BOOST_CHECK(v4.x() == ~1);
	BOOST_CHECK(v4.y() == ~2);
	BOOST_CHECK(v4.z() == ~3);

	v4 = !v;
	BOOST_CHECK(v4.x() == !1);
	BOOST_CHECK(v4.y() == !2);
	BOOST_CHECK(v4.z() == !3);

	v4 = 5;
	BOOST_CHECK(v4.x() == 5);
	BOOST_CHECK(v4.y() == 5);
	BOOST_CHECK(v4.z() == 5);

	v = {0, 1, 2};
	v4 = v && 2;
	BOOST_CHECK(v4.x() == 0);
	BOOST_CHECK(v4.y() == 1);
	BOOST_CHECK(v4.z() == 1);

	v4 = 2 && v;
	BOOST_CHECK(v4.x() == 0);
	BOOST_CHECK(v4.y() == 1);
	BOOST_CHECK(v4.z() == 1);

	v = {0, 1, 2};
	v4 = v || 2;
	BOOST_CHECK(v4.x() == 1);
	BOOST_CHECK(v4.y() == 1);
	BOOST_CHECK(v4.z() == 1);

	v4 = 2 || v;
	BOOST_CHECK(v4.x() == 1);
	BOOST_CHECK(v4.y() == 1);
	BOOST_CHECK(v4.z() == 1);
	return 0;
}
