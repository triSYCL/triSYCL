/* RUN: %{execute}%s

   Exercise buffer::set_final_data()
*/
#include <CL/sycl.hpp>

#include <numeric>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("construction from shared pointer", "[buffer]") {
  constexpr size_t N = 16;

  /** There is a draft for shared_ptr of arrays for C++17, but it is
      not here... So use a shared pointer with an explicit destructor
      waiting for 2017 */
  std::shared_ptr<int> data { new int[N], std::default_delete<int[]>{} };
  std::iota(data.get(), data.get() + N, 314);

  {
    buffer<int> b { data, N };

    queue {}.submit([&](handler &cgh) {
        auto kb = b.get_access<access::mode::read_write>(cgh);

        cgh.parallel_for<class generate>(range<1> { N },
                                         [=] (id<1> index) {
                                           kb[index] += 2015;
                                         });
      });
    /* Here the buffer b goes out of scope, so it first waits for
       kernel completion, then its destructor copies back its content
       to data memory */
  }

  for (int i = 0; i != N; ++i) {
    // std::cerr << data.get()[i] << ':' << i << std::endl;
    REQUIRE(data.get()[i] == i + 314 + 2015);
  }
}
