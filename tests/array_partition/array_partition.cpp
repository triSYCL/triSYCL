#include <array>
#include <CL/sycl.hpp>
#include <iostream>
#include <string>
#include <type_traits>

using namespace cl::sycl;

constexpr size_t SIZE = 4;

using Type = int;

template <typename Enum>
auto asInt (const Enum e) {
  return static_cast<typename std::underlying_type<Enum>::type>(e);
}

template <typename ArrayType>
void validateFunc (std::string name, ArrayType arr) {
  std::cout << name << std::endl;

  // Evaluate size
  std::cout << "Size: " << arr.size() << std::endl;

  // Show partition type
  std::cout << "Partition type: " << asInt(arr.get_partition_type())
            << std::endl;

  // Test iterator
  std::cout << "Content from iterator: ";
  for (auto i = arr.begin(); i != arr.end(); i++)
    std::cout << *i << " ";
  std::cout << "\n";

  // Test subscript operator
  std::cout << "Content from operator: ";
  for (auto e : arr)
    std::cout << e << " ";
  std::cout << "\n";
}

int main() {

  // Construct a partition_array from a list of number
  xilinx::partition_array<Type, SIZE> A = {1, 2, 3, 4};
  validateFunc("A", A);

  // Construct a partition_array from another partition_array of the same size
  xilinx::partition_array<Type, SIZE> B { A };
  validateFunc("B", B);

  // Construct a partition_array from a std::array
  std::array<Type, SIZE> toC = {5, 6, 7, 8};
  xilinx::partition_array<Type, SIZE> C { toC };
  validateFunc("C", C);

  // Construct a partition_array
  xilinx::partition_array<Type, SIZE> D;
  validateFunc("D", D);

  // Cyclic Partition for E
  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::cyclic<SIZE, 1>> E = {1, 2, 3, 4};
  validateFunc("E", E);

  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::cyclic<SIZE, 1>> F { E };
  validateFunc("F", F);

  F = C;

  validateFunc("FC", F);

  // Block Partition for G
  xilinx::partition_array<Type, SIZE,
                          xilinx::partition::block<SIZE, 1>> G;
  validateFunc("G", G);

  return 0;
}
