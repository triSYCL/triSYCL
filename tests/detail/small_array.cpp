/* RUN: %{execute}%s

   Test the internal small_array<> triSYCL class in trisycl::
   namespace by using #include "triSYCL/sycl.hpp"
*/

/// Test explicitly a feature of triSYCL, so include the triSYCL header
#include "triSYCL/sycl.hpp"

#include <boost/test/minimal.hpp>

/// Test explicitly a feature of triSYCL in ::trisycl namespace
using namespace trisycl;

struct sa1 : public detail::small_array<float, sa1, 1, true> {
  using detail::small_array<float, sa1, 1, true>::small_array;
};

struct sa2 : public detail::small_array<float, sa2, 2, true> {
  using detail::small_array<float, sa2, 2, true>::small_array;
};

struct sa3 : public detail::small_array<float, sa3, 3, true> {
  using detail::small_array<float, sa3, 3, true>::small_array;
};


/// Return true if both contents are the same
template <typename T, typename U, size_t N>
bool equal(const detail::small_array<T, U, N, true> &v,
           const std::array<T, N> &verif) {
  /* Do not use directly v == verif because we want to verify the
     (implicit) constructor itself */
  auto p = std::begin(verif);
  for (auto e : v)
    if (e != *p++)
      return false;

  return true;
}


int test_main(int argc, char *argv[]) {
  sa1 a;
  sa1 a1 { 3 };
  BOOST_CHECK(a1[0] == 3);
  a = a1;
  std::cerr << a[0] << std::endl;
  BOOST_CHECK(a[0] == 3);
  auto a2 = a*a1;
  BOOST_CHECK(a2[0] == 9);
  sa2 b1 { 1, 2 };
  BOOST_CHECK(equal(b1, { 1, 2 }));
  sa2 b2 = { 1, 2 };
  BOOST_CHECK(b1 == b2);
  sa2 b3 = b2 + b1;
  BOOST_CHECK(equal(b3, { 2, 4 }));
  sa2 b4 { b1 - b3 };
  BOOST_CHECK(equal(b4, { -1, -2 }));
  auto b5 = b4/b1;
  BOOST_CHECK(equal(b5, { -1, -1 }));
  sa3 c = { 1, 2, 3 };
  BOOST_CHECK(equal(c, { 1, 2, 3 }));
  std::array<float, 3> da = { 2, 3, 4};
  sa3 d = da;
  BOOST_CHECK(equal(d , { 2, 3, 4 }));

  return 0;
}
