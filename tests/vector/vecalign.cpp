/* RUN: %{execute}%s

   Test some vec<> as method behaviour
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
	auto val = std::alignment_of<cl::sycl::vec<char, 2>>::value;
	BOOST_CHECK(val == 2);
	val = std::alignment_of<cl::sycl::vec<char, 3>>::value;
	BOOST_CHECK(val == 4);
	val = std::alignment_of<cl::sycl::vec<char, 4>>::value;
	BOOST_CHECK(val == 4);

	val = std::alignment_of<cl::sycl::vec<int, 2>>::value;
	BOOST_CHECK(val == 8);
	val = std::alignment_of<cl::sycl::vec<int, 3>>::value;
	BOOST_CHECK(val == 16);
	val = std::alignment_of<cl::sycl::vec<int, 4>>::value;
	BOOST_CHECK(val == 16);

	return 0;
}
