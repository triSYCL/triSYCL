/* RUN: %{execute}%s

   Exercise triSYCL sycl::vendor::trisycl::random::xorshift extension
*/
#include <triSYCL/vendor/trisycl/random/xorshift.hpp>
#include <iostream>
#include <type_traits>

#include <boost/test/minimal.hpp>

namespace r = trisycl::vendor::trisycl::random;

int test_main(int argc, char *argv[]) {

  r::xorshift rng32;
  // Test default size
  static_assert(std::is_same_v<decltype(rng32), r::xorshift<32>>);
  for(int i = 0; i != 20; ++i)
    std::cout << "xor32\t" << i << ":\t" << std::hex << rng32() << std::endl;

  r::xorshift<64> rng64;
  for(int i = 0; i != 20; ++i)
    std::cout << "xor64\t" << i << ":\t" << std::hex << rng64() << std::endl;

  r::xorshift<128> rng128;
  for(int i = 0; i != 20; ++i) {
    auto [ a, b, c, d ] = rng128();
    std::cout << "xor128\t" << i << ":\t" << std::hex << a << '\t' << b << '\t'
              << c << '\t' << d << '\t' << std::endl;
  }
  return 0;
}
