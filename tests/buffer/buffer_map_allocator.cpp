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

  {
    cl::sycl::buffer<int, 1, cl::sycl::map_allocator<int> > dest_buf
      { vect.data(), cl::sycl::range<1>(N) };
  }

  for (int i = 0; i < N; ++i) {
    // std::cerr << vect[i] << ':' << i << std::endl;
    BOOST_CHECK(vect[i] == 0);
  }

  return 0;
}
