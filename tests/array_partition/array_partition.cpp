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
using namespace cl::sycl::vendor;

constexpr size_t ARRAY_LENGTH = 4;

using Type = int;

#define TRISYCL_CHECK(ARRAY, PARTITION_TYPE, EXPECTED_VALUE)            \
  {                                                                     \
    std::array<Type, ARRAY_LENGTH> expected_value EXPECTED_VALUE;       \
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
  xilinx::partition_array<Type, ARRAY_LENGTH> A = {1, 2, 3, 4};
  TRISYCL_CHECK(A, xilinx::partition::type::none, ({1, 2, 3, 4}))

  // Construct a partition_array from another partition_array of the same size
  xilinx::partition_array<Type, ARRAY_LENGTH> B { A };
  TRISYCL_CHECK(B, xilinx::partition::type::none, ({1, 2, 3, 4}));

  // Construct a partition_array from a std::array
  std::array<Type, ARRAY_LENGTH> toC = {5, 6, 7, 8};
  xilinx::partition_array<Type, ARRAY_LENGTH> C { toC };
  TRISYCL_CHECK(C, xilinx::partition::type::none, ({5, 6, 7, 8}));

  // Construct a partition_array by default
  xilinx::partition_array<Type, ARRAY_LENGTH> D;
  D = B;
  TRISYCL_CHECK(D, xilinx::partition::type::none, ({1, 2, 3, 4}));

  // Cyclic Partition for E
  xilinx::partition_array<Type, ARRAY_LENGTH,
                          xilinx::partition::cyclic<ARRAY_LENGTH>> E = {1, 2, 3, 4};
  TRISYCL_CHECK(E, xilinx::partition::type::cyclic, ({1, 2, 3, 4}));

  xilinx::partition_array<Type, ARRAY_LENGTH,
                          xilinx::partition::cyclic<ARRAY_LENGTH>> F { E };
  TRISYCL_CHECK(F, xilinx::partition::type::cyclic, ({1, 2, 3, 4}));

  F = C;

  TRISYCL_CHECK(F, xilinx::partition::type::cyclic, ({5, 6, 7, 8}));

  // Block Partition for G
  xilinx::partition_array<Type, ARRAY_LENGTH,
                          xilinx::partition::block<ARRAY_LENGTH>> G = F;
  TRISYCL_CHECK(G, xilinx::partition::type::block, ({5, 6, 7, 8}));

  // Block Partition for H
  xilinx::partition_array<Type, ARRAY_LENGTH,
                          xilinx::partition::complete<>> H;
  H = toC;
  TRISYCL_CHECK(H, xilinx::partition::type::complete, ({5, 6, 7, 8}));

  return 0;
}
