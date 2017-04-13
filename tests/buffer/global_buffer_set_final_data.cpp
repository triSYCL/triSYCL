/* RUN: %{execute}%s

   Exercise buffer::set_final_data() with a global buffer
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;


constexpr size_t N = 16;

buffer<int> buf{ N };

int test_main(int /*argc*/, char*[] /*argv*/) {
  /** There is a draft for shared_ptr of arrays for C++17, but it is
      not here... So use a shared pointer with an explicit destructor
      waiting for 2017 */
  std::shared_ptr<int> result { new int[N], std::default_delete<int[]>{} };

  /* Note that here there is a nice implicit conversion happening
     from std::shared_ptr<int> to std::weak_ptr<int> */
  buf.set_final_data(result);

  queue {}.submit([&](handler &cgh) {
      auto a = buf.get_access<access::mode::write>(cgh);

      cgh.parallel_for<class generate>(range<1> { N },
                                       [=] (id<1> index) {
                                         a[index] = index;
                                       });
    });
  /* So since we want the buffer destructor of b to copy back the
     content just before using result, just reassign an empty buffer
     to b
  */
  buf = {};
  for (int i = 0; i != N; ++i) {
    // std::cerr << result.get()[i] << ':' << i << std::endl;
    BOOST_CHECK(result.get()[i] == i);
  }

  return 0;
}
