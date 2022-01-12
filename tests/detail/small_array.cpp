/* RUN: %{execute}%s

   Test the internal small_array<> triSYCL class in trisycl::
   namespace by using #include "triSYCL/sycl.hpp"
*/


/// Test explicitly a feature of triSYCL, so include the triSYCL header
#include "triSYCL/sycl.hpp"

#include <catch2/catch_test_macros.hpp>

/// Test explicitly a feature of triSYCL in ::trisycl namespace
using namespace trisycl;

struct sa1 : public detail::small_array<float, sa1, 1> {
  using detail::small_array<float, sa1, 1>::small_array;
};

struct sa2 : public detail::small_array<float, sa2, 2> {
  using detail::small_array<float, sa2, 2>::small_array;
};

struct sa3 : public detail::small_array<float, sa3, 3> {
  using detail::small_array<float, sa3, 3>::small_array;
};

/// \todo just use range equal algorithm
/// Return true if both contents are the same
template <typename T, typename U, size_t N>
bool equal(const detail::small_array<T, U, N> &v,
           const std::array<T, N> &verif) {
  /* Do not use directly v == verif because we want to verify the
     (implicit) constructor itself */
  auto p = std::begin(verif);
  for (auto e : v)
    if (e != *p++)
      return false;

  return true;
}


TEST_CASE("small_array", "[detail]") {
  sa1 a;
  sa1 a1 { 3 };
  REQUIRE(a1[0] == 3);
  a = a1;
  std::cerr << a[0] << std::endl;
  REQUIRE(a[0] == 3);
  auto a2 = a*a1;
  REQUIRE(a2[0] == 9);
  sa2 b1 { 1, 2 };
  REQUIRE(equal(b1, { 1, 2 }));
  sa2 b2 = { 1, 2 };
  REQUIRE(b1 == b2);
  sa2 b3 = b2 + b1;
  REQUIRE(equal(b3, { 2, 4 }));
  sa2 b4 { b1 - b3 };
  REQUIRE(equal(b4, { -1, -2 }));
  auto b5 = b4/b1;
  REQUIRE(equal(b5, { -1, -1 }));
  sa3 c = { 1, 2, 3 };
  REQUIRE(equal(c, { 1, 2, 3 }));
  std::array<float, 3> da = { 2, 3, 4};
  sa3 d = da;
  REQUIRE(equal(d , { 2, 3, 4 }));
}
