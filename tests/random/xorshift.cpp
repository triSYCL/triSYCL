/* RUN: %{execute}%s

   Exercise triSYCL sycl::vendor::trisycl::random::xorshift extension
*/
#include <iostream>
#include <type_traits>

#include <triSYCL/vendor/trisycl/random/xorshift.hpp>

#include <boost/test/minimal.hpp>

namespace r = trisycl::vendor::trisycl::random;

int test_main(int argc, char *argv[]) {

  r::xorshift rng32;
  // Test default size
  static_assert(std::is_same_v<decltype(rng32), r::xorshift<32>>);
  BOOST_CHECK(rng32() == 0xb44f687c);
  // for(int i = 0; i != 20; ++i)
  //  std::cout << "xor32\t" << i << ":\t" << std::hex << rng32() << std::endl;

  r::xorshift<64> rng64;
  BOOST_CHECK(rng64() == 0x79690975fbde15b0);
  // for(int i = 0; i != 20; ++i)
  //  std::cout << "xor64\t" << i << ":\t" << std::hex << rng64() << std::endl;

  r::xorshift<128> rng128;
  BOOST_CHECK((rng128() == r::xorshift<128>::value_type
      { 0x159a55e5, 0x1f123bb5, 0x5491333, 0xdca345ea }));
  // for(int i = 0; i != 20; ++i) {
  //   auto [ a, b, c, d ] = rng128();
  //   std::cout << "xor128\t" << i << ":\t" << std::hex << a << '\t'
  //               << b << '\t' << c << '\t' << d << '\t' << std::endl;
  // }

  r::xorshift<32> rng32_0 { 1 };
  BOOST_CHECK(rng32_0() == 3 );

  r::xorshift<64> rng64_0 { 1 };
  BOOST_CHECK(rng64_0() == 0x40822041 );

  r::xorshift<128> rng128_0 { { 1, 2, 3, 4 } };
  BOOST_CHECK((rng128_0() == r::xorshift<128>::value_type { 2, 3, 4, 0x80d }));
  return 0;
}
