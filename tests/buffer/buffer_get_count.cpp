/* RUN: %{execute}%s

   Exercise buffer size-like methods
*/

#include <CL/sycl.hpp>
#include <boost/test/minimal.hpp>
#include <iostream>

using namespace cl::sycl;

#define CHECK_RANGE_BUFFER(r, b)                                  \
  BOOST_CHECK(r  == b.get_range());                               \
  BOOST_CHECK(r.get_count() == b.get_count());                    \
  BOOST_CHECK(b.get_size()                                        \
              == b.get_count()*sizeof(decltype(b)::value_type));


int test_main(int argc, char *argv[]) {
	std::vector<int> v(10);
	size_t bufferSize = std::distance(v.begin(), v.end()); 
	std::shared_ptr<int> ptr{new int[bufferSize]};
	std::copy(v.begin(), v.end(), ptr.get());
	buffer<int, 1> buf(ptr, range<1>(bufferSize));

	if (buf.get_count() != v.size())
	{
		std::cout << "v.size() = " << v.size() << std::endl;
		std::cout << "buf.get_count() = " << buf.get_count() << std::endl;
		assert(false);
	}
	return 0;
}
