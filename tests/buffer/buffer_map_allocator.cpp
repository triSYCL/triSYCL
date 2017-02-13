/* RUN: %{execute}%s

   Exercise buffer::set_final_data()
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {

  constexpr size_t N = 16;

  std::vector<int> vect(N, 42);

  cl::sycl::buffer<int, 1, cl::sycl::map_allocator<int> > buff
    { vect.data(), cl::sycl::range<1>(N) };

  auto read = buff.get_access
    <sycl::access::mode::read, sycl::access::target::host_buffer>();
  for (int i = 0; i < N; ++i) {
    // std::cerr << vect[i] << ':' << i << std::endl;
    // check that the buffer has been properly 're-initialized'
    BOOST_CHECK(read[i] == 0);
    // check the buffer's location
    BOOST_CHECK(&vect[i] == &read[i]);
  }

  return 0;
}
