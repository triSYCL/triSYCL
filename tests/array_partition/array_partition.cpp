/* Test that the partition_array works on normal host code

   RUN: %{execute}%s
*/

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <type_traits>

#include <boost/test/minimal.hpp>

#include <CL/sycl.hpp>

using namespace cl::sycl;
using namespace cl::sycl::xilinx::partition;

constexpr size_t SIZE = 4;

using Type = int;

#define TRISYCL_CHECK(ARRAY, PARTITION_TYPE, EXPECTED_VALUE)            \
  {                                                                     \
    std::array<Type, SIZE> expected_value EXPECTED_VALUE;               \
    BOOST_CHECK(decltype(ARRAY)::partition_type == PARTITION_TYPE);     \
    /* Evaluate size */                                                 \
    BOOST_CHECK(ARRAY.size() == expected_value.size());                 \
    /* Test subscript operator */                                       \
    for (std::size_t i = 0; i != ARRAY.size(); ++i)                     \
      BOOST_CHECK(ARRAY[i] == expected_value[i]);                       \
    /* Test iterator */                                                 \
    BOOST_CHECK(std::equal(ARRAY.begin(), ARRAY.end(),                  \
                           expected_value.begin(), expected_value.end())); \
  }

int test_main(int argc, char *argv[]) {
  // Construct a partition_array from a list of number
  xilinx::partition_array<Type, SIZE> A = {1, 2, 3, 4};
  TRISYCL_CHECK(A, type::none, ({1, 2, 3, 4}))

  // Construct a partition_array from another partition_array of the same size
  xilinx::partition_array<Type, SIZE> B { A };
  TRISYCL_CHECK(B, type::none, ({1, 2, 3, 4}));

  // Construct a partition_array from a std::array
  std::array<Type, SIZE> toC = {5, 6, 7, 8};
  xilinx::partition_array<Type, SIZE> C { toC };
  TRISYCL_CHECK(C, type::none, ({5, 6, 7, 8}));

  // Construct a partition_array by default
  xilinx::partition_array<Type, SIZE> D;
  D = B;
  TRISYCL_CHECK(D, type::none, ({1, 2, 3, 4}));

  // Cyclic Partition for E
  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::cyclic<SIZE, 1>> E = {1, 2, 3, 4};
  TRISYCL_CHECK(E, type::cyclic, ({1, 2, 3, 4}));

  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::cyclic<SIZE, 1>> F { E };
  TRISYCL_CHECK(F, type::cyclic, ({1, 2, 3, 4}));

  F = C;

  TRISYCL_CHECK(F, type::cyclic, ({5, 6, 7, 8}));

  // Block Partition for G
  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::block<SIZE, 1>> G = F;
  TRISYCL_CHECK(G, type::block, ({5, 6, 7, 8}));

  // Block Partition for H
  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::complete<>> H;
  H = toC;
  TRISYCL_CHECK(H, type::complete, ({5, 6, 7, 8}));

  return 0;
}
