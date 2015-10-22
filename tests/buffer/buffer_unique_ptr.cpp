/* RUN: %{execute}%s

   Exercise buffer construction from unique pointer
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {

  constexpr size_t N = 16;

  // Allocate some memory to test ownership give-away
  std::unique_ptr<int[]> init { new int[N] };
  std::iota(init.get(), init.get() + N, 314);

  buffer<int> a { std::move(init), N };
  // After buffer construction, init has been deallocated
  BOOST_CHECK(!init);

  buffer<int> b { N };

  queue {}.submit([&](handler &cgh) {
      auto ka = a.get_access<access::read>(cgh);
      auto kb = b.get_access<access::write>(cgh);

      cgh.parallel_for<class update>(range<1> { N },
                                     [=] (id<1> index) {
                                       kb[index] = ka[index]*2;
                                     });
    });

  auto result = b.get_access<access::read, access::host_buffer>();
  for (int i = 0; i != N; ++i) {
    //std::cerr << result[i] << ':' << i << std::endl;
    BOOST_CHECK(result[i] == 2*(314 + i));
  }

  return 0;
}
