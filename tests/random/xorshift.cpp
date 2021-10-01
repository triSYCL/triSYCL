/* RUN: %{execute}%s

   Exercise triSYCL sycl::vendor::trisycl::random::xorshift extension
*/
#include <iostream>
#include <random>
#include <type_traits>

#include <triSYCL/vendor/triSYCL/random/xorshift.hpp>

#include <catch2/catch_test_macros.hpp>

namespace r = trisycl::vendor::trisycl::random;

TEST_CASE("xorshift", "[random]") {
  r::xorshift rng32;
  // Test default size
  static_assert(std::is_same_v<decltype(rng32), r::xorshift<32>>);
  REQUIRE(rng32() == 0xb44f687c);
  // for(int i = 0; i != 20; ++i)
  //  std::cout << "xor32\t" << i << ":\t" << std::hex << rng32() << std::endl;

  r::xorshift<64> rng64;
  REQUIRE(rng64() == 0x79690975fbde15b0);
  // for(int i = 0; i != 20; ++i)
  //  std::cout << "xor64\t" << i << ":\t" << std::hex << rng64() << std::endl;

  r::xorshift<128> rng128;
  REQUIRE((rng128() == r::xorshift<128>::value_type
      { 0x159a55e5, 0x1f123bb5, 0x5491333, 0xdca345ea }));
  // for(int i = 0; i != 20; ++i) {
  //   auto [ a, b, c, d ] = rng128();
  //   std::cout << "xor128\t" << i << ":\t" << std::hex << a << '\t'
  //               << b << '\t' << c << '\t' << d << '\t' << std::endl;
  // }

  r::xorshift<32> rng32_0 { 1 };
  REQUIRE(rng32_0() == 3 );

  r::xorshift<64> rng64_0 { 1 };
  REQUIRE(rng64_0() == 0x40822041 );

  r::xorshift<128> rng128_0 { { 1, 2, 3, 4 } };
  REQUIRE((rng128_0() == r::xorshift<128>::value_type { 2, 3, 4, 0x80d }));

  // To generate integer values between 0 and 9
  std::uniform_int_distribution<int> dist { 0, 9 };
  REQUIRE(dist(rng32) == 8);
  REQUIRE(dist(rng32) == 3);

  std::uniform_real_distribution<float> rdist { 0, 1 };
  for (int n = 0; n < 10; ++n)
        std::cout << rdist(rng32) << '\n';
}
