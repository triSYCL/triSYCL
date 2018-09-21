/* RUN: %{execute}%s

   Try a map_allocator<int>
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {

  constexpr auto N = 16;

  std::vector<int> vect(N, 42);

  buffer<int, 1, map_allocator<int>> buff { vect.data(), range<1> { size_t(N) } };

  auto read = buff.get_access<access::mode::read>();
  for (auto i = 0; i < N; ++i) {
    // std::cerr << vect[i] << ':' << i << std::endl;
    // check that the buffer has been properly 're-initialized'
    BOOST_CHECK(read[i] == 42);
    // check the buffer's location
    BOOST_CHECK(&vect[i] == &read[i]);
  }

  return 0;
}
