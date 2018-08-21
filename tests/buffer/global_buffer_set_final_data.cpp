/* RUN: %{execute}%s

   Exercise buffer::set_final_data() with a global buffer
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;


constexpr size_t N = 16;

buffer<int> b { N };

int test_main(int argc, char *argv[]) {
  /** There is a draft for shared_ptr of arrays for C++17, but it is
      not here... So use a shared pointer with an explicit destructor
      waiting for 2017 */
  std::shared_ptr<int> result { new int[N], std::default_delete<int[]>{} };

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
  /* Be careful with global buffers with copy back behaviour, since
     the destructor is called at program exit (typically in some C++
     library at_exit handler at a time the object to copy to may be
     dead for a while, so with a lot of havoc and hard debug time...

     So since we want the buffer destructor of b to copy back the
     content just before using result, just reassign an empty buffer
     to b
  */
  b = {};
  for (int i = 0; i != N; ++i) {
    // std::cerr << result.get()[i] << ':' << i << std::endl;
    BOOST_CHECK(result.get()[i] == i);
  }

  return 0;
}
