/* RUN: %{execute}%s | %{filecheck} %s

   Example using extension for SYCL objects with dimensions higher than 3
*/

// Allows also ranks higher than 3
#define TRISYCL_ALLOW_ANY_DIMENSION

#include <sycl/sycl.hpp>

#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("use dimensions higher than 3", "[id]") {
  // CTAD
  sycl::id i5 { 0, 1, 2, 3, 4 };
  i5.display();

  sycl::id<4> i4;
  i4 = { 1, 2, 3, 4 };
  i4.display();
  i4 += { 1, 2, 4, 8 };
  i4.display();

  sycl::range<5> r { 1, 2, 3, 4, 5 };
  sycl::id<5> ir { r };
  ir.display();

  sycl::item<6> it { { 1, 2, 3, 4, 5, 6 },
               { 7, 8, 9, 10, 11, 12 },
               { 13, 14, 15, 16, 17 } };
  sycl::id iditem { it };
  iditem.display();
}
