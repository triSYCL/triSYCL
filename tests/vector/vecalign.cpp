/* RUN: %{execute}%s

   Test some vec<> alignment
*/
#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

TEST_CASE("alignment", "[vector]") {
  auto val = std::alignment_of<cl::sycl::vec<char, 2>>::value;
  REQUIRE(val == 2);
  val = std::alignment_of<cl::sycl::vec<char, 3>>::value;
  REQUIRE(val == 4);
  val = std::alignment_of<cl::sycl::vec<char, 4>>::value;
  REQUIRE(val == 4);

  val = std::alignment_of<cl::sycl::vec<int, 2>>::value;
  REQUIRE(val == 8);
  val = std::alignment_of<cl::sycl::vec<int, 3>>::value;
  REQUIRE(val == 16);
  val = std::alignment_of<cl::sycl::vec<int, 4>>::value;
  REQUIRE(val == 16);
}
