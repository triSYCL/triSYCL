/* RUN: %{execute}%s

   Exercise buffer construction from unique pointer
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {

  constexpr size_t N = 16;

  /* Allocate some memory to test ownership give-away

     Interestingly, we cannot have directly
     \code
     std::unique_ptr<int[]> init { new int[N] };
     \endcode
     because there is the buffer constructor take a \c
     std::shared_ptr<T> and in C++17 there is an implicit conversion
     from \c std::unique_ptr<T[]> to \c std::shared_ptr<T[]> and
     another one \c std::shared_ptr<T[]> to \c std::shared_ptr<T>, but
     globally it requires chaining more than 1 implicit conversion and
     thus it is not tried.

     Should we provide a \c std::unique_ptr<T[]> for buffer in the
     SYCL specification?

     Or is there a missing \c std::shared_ptr<T> constructor from a \c
     std::unique_ptr<T[]> in C++17?
  */
  std::unique_ptr<int, std::default_delete<int[]>> init { new int[N] };
  std::iota(init.get(), init.get() + N, 314);

  buffer<int> a { std::move(init), N };
  // After buffer construction, init has been deallocated
  BOOST_CHECK(!init);

  buffer<int> b { N };

  queue {}.submit([&](handler &cgh) {
      auto ka = a.get_access<access::mode::read>(cgh);
      auto kb = b.get_access<access::mode::write>(cgh);

      cgh.parallel_for<class update>(range<1> { N },
                                     [=] (id<1> index) {
                                       kb[index] = ka[index]*2;
                                     });
    });

  auto result = b.get_access<access::mode::read>();
  for (int i = 0; i != N; ++i) {
    //std::cerr << result[i] << ':' << i << std::endl;
    BOOST_CHECK(result[i] == 2*(314 + i));
  }

  return 0;
}
