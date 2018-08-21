/* RUN: %{execute}%s

   Exercise buffer::set_final_data()
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {

  constexpr size_t N = 16;

  /** There is a draft for shared_ptr of arrays for C++17, but it is
      not here... So use a shared pointer with an explicit destructor
      waiting for 2017 */
  std::shared_ptr<int> result { new int[N], std::default_delete<int[]>{} };

  {
    buffer<int> b { N };
    /* Note that here there is a nice implicit conversion happening
       from std::shared_ptr<int> to std::weak_ptr<int> */
    b.set_final_data(result);

    queue {}.submit([&](handler &cgh) {
        auto a = b.get_access<access::mode::write>(cgh);

        cgh.parallel_for<class generate>(range<1> { N },
                                         [=] (id<1> index) {
                                           a[index] = index[0];
                                         });
      });
    // auto B = b.get_access<access::read, access::host_buffer>();
    // for (int i = 0; i != N; ++i)
    // std::cerr << B[i] << ':' << i << std::endl;

    /* Here the buffer b goes out of scope, so it first waits for
       kernel completion, then its destructor copies back its content
       to result as instructed by set_final_data(result) */
  }

  for (int i = 0; i != N; ++i) {
    // std::cerr << result.get()[i] << ':' << i << std::endl;
    BOOST_CHECK(result.get()[i] == i);
  }

  return 0;
}
