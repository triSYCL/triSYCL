/* RUN: %{execute}%s

   Test some vec<> as method behaviour
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
	vec<float, 8> v = {1.0f, 2.5f, 3.2f, 4.8f, 0.0f, -1.0f, -2.5f, -4.8f};

	BOOST_CHECK(v[0] == 1.0f);
	BOOST_CHECK(v[1] == 2.5f);
	BOOST_CHECK(v[2] == 3.2f);
	BOOST_CHECK(v[3] == 4.8f);
	BOOST_CHECK(v[4] == 0.0f);
	BOOST_CHECK(v[5] == -1.0f);
	BOOST_CHECK(v[6] == -2.5f);
	BOOST_CHECK(v[7] == -4.8f);

	auto v2 = v.convert<int, cl::sycl::rounding_mode::automatic>();
	BOOST_CHECK(v2[0] == 1);
	BOOST_CHECK(v2[1] == 2);
	BOOST_CHECK(v2[2] == 3);
	BOOST_CHECK(v2[3] == 4);
	BOOST_CHECK(v2[4] == 0);
	BOOST_CHECK(v2[5] == -1);
	BOOST_CHECK(v2[6] == -2);
	BOOST_CHECK(v2[7] == -4);
	return 0;
}
