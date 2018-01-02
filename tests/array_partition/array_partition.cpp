#include <CL/sycl.hpp>
#include <array>
#include <string>
#include <type_traits>
#include <iostream>

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
  std::cout << "Partition type: " << asInt(arr.getParType()) << std::endl;

  // Test iterator
  std::cout << "Content from iterator: ";
  for (auto i = arr.begin(); i != arr.end(); i++)
    std::cout << *i << " ";
  std::cout << "\n";

  // Test subscript operator
  std::cout << "Content from operator: ";
  for (int i = 0; i < arr.size(); i++)
    std::cout << arr[i] << " ";
  std::cout << "\n";
}

int main() {

  // Construct a array from a list of number
  vendor::array<Type, SIZE> A = {1, 2, 3, 4};
  validateFunc("A", A);

  // Construct a array from another array of the same size
  vendor::array<Type, SIZE> B(A);
  validateFunc("B", B);

  // Construct a array from a std::array
  std::array<Type, SIZE> toC = {5, 6, 7, 8};
  vendor::array<Type, SIZE> C(toC);
  validateFunc("C", C);

  // Construct a array
  vendor::array<Type, SIZE> D;
  validateFunc("D", D);

  // Cyclic Partition for E
  vendor::array<Type, SIZE, SIZE, 1,
                vendor::partition::par_type::cyclic> E = {1, 2, 3, 4};
  validateFunc("E", E);

  vendor::array<Type, SIZE, SIZE, 1,
                vendor::partition::par_type::cyclic> F(E);
  validateFunc("F", F);

  // Block Partition for G
  vendor::array<Type, SIZE, SIZE, 1, vendor::partition::par_type::block> G;
  validateFunc("G", G);

  return 0;
}
