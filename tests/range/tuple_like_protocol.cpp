/* RUN: %{execute}%s | %{filecheck} %s

   Check tuple-like protocol of sycl::range
*/

#include <sycl/sycl.hpp>

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <tuple>
#include <type_traits>

TEST_CASE("test the tuple-like protocol", "[range]") {
  // CTAD
  sycl::range i { 1, 2, 3 };
  REQUIRE(std::get<0>(i) == 1);
  REQUIRE(std::get<1>(i) == 2);
  REQUIRE(std::get<2>(i) == 3);
  static_assert(
      std::is_same_v<std::size_t, std::tuple_element_t<0, decltype(i)>>);
  static_assert(
      std::is_same_v<std::size_t, std::tuple_element_t<1, decltype(i)>>);
  static_assert(
      std::is_same_v<std::size_t, std::tuple_element_t<2, decltype(i)>>);
  {
    auto [x, y, z] = i;
    REQUIRE(x == 1);
    REQUIRE(y == 2);
    REQUIRE(z == 3);
    ++x;
    --y;
    ++z;
    REQUIRE((i == sycl::range { 1, 2, 3 }));
  }
  {
    auto& [x, y, z] = i;
    REQUIRE(x == 1);
    REQUIRE(y == 2);
    REQUIRE(z == 3);
    ++x;
    --y;
    ++z;
    REQUIRE((i == sycl::range { 2, 1, 4 }));
  }
}
