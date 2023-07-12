/* RUN: %{execute}%s

  Test the behavior of vector math functions.

*/

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>

#include <sycl/sycl.hpp>

#include <catch2/catch_test_macros.hpp>

template <typename T, int Dim>
void do_vec_unary_math(sycl::vec<T, Dim> v) {
  auto abs = sycl::abs(v);
  auto clamp = sycl::clamp(v, 2.0f, 4.0f);
  auto floor = sycl::floor(v);
  auto length = sycl::length(v);
  auto normalize = sycl::normalize(v);

  T len {0};
  for (int i = 0; i < Dim; ++i) {
    len += std::pow(v[i], 2);
  }

  len = std::sqrt(len);

  REQUIRE(length == len);

  for (int i = 0; i < Dim; ++i) {
    REQUIRE(abs[i] == std::abs(v[i]));
    REQUIRE(clamp[i] == std::clamp(v[i], 2.0f, 4.0f));
    REQUIRE(floor[i] == std::floor(v[i]));
    REQUIRE(normalize[i] == v[i] / len);
  }
}

// only intended for vecs of equal size, similar to the implemented math
// functions
template <typename T, int Dim>
void do_vec_binary_math(sycl::vec<T, Dim> v, sycl::vec<T, Dim> v2) {
  auto fmin = sycl::fmin(v, v2);
  auto fmax = sycl::fmax(v, v2);
  auto min = sycl::min(v, v2);
  auto max = sycl::max(v, v2);

  for (int i = 0; i < Dim; ++i) {
    REQUIRE(fmin[i] == std::fmin(v2[i], v[i]));
    REQUIRE(fmax[i] == std::fmax(v[i], v2[i]));
    REQUIRE(min[i] == std::min(v2[i], v[i]));
    REQUIRE(max[i] == std::max(v[i], v2[i]));
  }
}

inline void do_check_sign() {
  sycl::float8 input{std::numeric_limits<float>::quiet_NaN(),
                     std::numeric_limits<float>::signaling_NaN(),
                     std::numeric_limits<float>::infinity(),
                     -1. * std::numeric_limits<float>::infinity(),
                     std::numeric_limits<float>::lowest(),
                     std::numeric_limits<float>::max(),
                     0.,
                     -0.};

  auto res = sycl::sign(input);
  float one = 1.;
  float mone = -1.;
  std::array<uint32_t, 8> expected_output{0,
                                          0,
                                          reinterpret_cast<uint32_t &>(one),
                                          reinterpret_cast<uint32_t &>(mone),
                                          reinterpret_cast<uint32_t &>(mone),
                                          reinterpret_cast<uint32_t &>(one),
                                          0,
                                          uint32_t{1} << 31};

  for (std::size_t i = 0; i < 8; ++i) {
    std::cerr << i << ' ' << res[i] << ' ' << expected_output[i] << std::endl;
    REQUIRE(reinterpret_cast<uint32_t &>(res[i]) == expected_output[i]);
  }
}

TEST_CASE("vector operations", "[math]") {
  sycl::float2 f2 {2.3f, 4.1f};
  sycl::float3 f3_a {1,2,3};
  sycl::float3 f3_b {1,5,7};
  sycl::float3 f3_c {3.0,1.0,2.0};
  sycl::float4 f4_a {1,2,3,2};
  sycl::float4 f4_b {1,5,7,4};
  sycl::float8 f8 {-5.0f,-5.1f,-5.9f,-6.9f,-6.1f,5.0f,5.1f,5.9f};
  sycl::float16 f16_a {10.0f};
  sycl::float16 f16_b {15};

  do_vec_unary_math(f2);
  do_vec_unary_math(f3_a);
  do_vec_unary_math(f3_b);
  do_vec_unary_math(f3_c);
  do_vec_unary_math(f4_a);
  do_vec_unary_math(f4_b);
  do_vec_unary_math(f8);
  do_vec_unary_math(f16_a);
  do_vec_unary_math(f16_b);

  do_vec_binary_math(f3_a, f3_b);
  // do_vec_binary_math(f3_a, f3_c);
  // do_vec_binary_math(f4_a, f4_b);
  // do_vec_binary_math(f16_a, f16_b);

  // not as trivially testable generically as the above
  auto cross3_ab = sycl::cross(f3_a, f3_b);
  auto cross4_ab = sycl::cross(f4_a, f4_b);

  REQUIRE(cross3_ab.x() == -1);
  REQUIRE(cross3_ab.y() == -4);
  REQUIRE(cross3_ab.z() == 3);

  REQUIRE(cross4_ab.x() == -1);
  REQUIRE(cross4_ab.y() == -4);
  REQUIRE(cross4_ab.z() == 3);
  REQUIRE(cross4_ab.w() == 0);

  auto dot3_ab = sycl::dot(f3_a, f3_b);
  auto dot4_ab = sycl::dot(f4_a, f4_b);;
  auto dot16_ab = sycl::dot(f16_a, f16_b);;

  REQUIRE(dot3_ab == 32);
  REQUIRE(dot4_ab == 40);
  REQUIRE(dot16_ab == 2400);

  do_check_sign();
}
