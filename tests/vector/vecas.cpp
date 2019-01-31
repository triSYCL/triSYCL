/* RUN: %{execute}%s

   Test some vec<> as method behaviour
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
	vec<int, 4> v = {1, 2, 3, 4};

	BOOST_CHECK(v.x() == 1);
	BOOST_CHECK(v.y() == 2);
	BOOST_CHECK(v.z() == 3);
	BOOST_CHECK(v.w() == 4);

	auto v2 = v.as<cl::sycl::vec<unsigned int, 4>>();
	BOOST_CHECK(v2.x() == 1);
	BOOST_CHECK(v2.y() == 2);
	BOOST_CHECK(v2.z() == 3);
	BOOST_CHECK(v2.w() == 4);

	auto v3 = v.as<cl::sycl::vec<unsigned char, 16>>();
	BOOST_CHECK(v3[0] == 1);
	BOOST_CHECK(v3[4] == 2);
	BOOST_CHECK(v3[8] == 3);
	BOOST_CHECK(v3[12] == 4);

	vec<float, 4> vf = { 1.0, 2.0, 3.0, 4.0 };
	auto vi = vf.as<cl::sycl::vec<unsigned int, 4>>();
	BOOST_CHECK(vi[0] == 0x3f800000);
	BOOST_CHECK(vi[1] == 0x40000000);
	BOOST_CHECK(vi[2] == 0x40400000);
	BOOST_CHECK(vi[3] == 0x40800000);
	return 0;
}
