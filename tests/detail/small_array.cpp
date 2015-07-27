#include "CL/sycl/detail/small_array.hpp"

using namespace cl::sycl;

struct sa1 : public detail::small_array<float, sa1, 1, true> {
  using detail::small_array<float, sa1, 1, true>::small_array;
};

struct sa2 : public detail::small_array<float, sa2, 2, true> {
  using detail::small_array<float, sa2, 2, true>::small_array;
};

struct sa3 : public detail::small_array<float, sa3, 3, true> {
  using detail::small_array<float, sa3, 3, true>::small_array;
};

int main() {
  sa1 a;
  sa1 a1 { 1 };
  a = a1;
  auto a2 = a*a1;
  sa2 b1 { 1, 2 };
  sa2 b2 = { 1, 2 };
  sa2 b3 = b1;
  b2 + b3;
  sa2 b4 { b1 + b2 };
  auto b5 = b4/b1;
  sa3 c = { 1, 2, 3 };
  std::array<float, 3> da = { 2, 3, 4};
  sa3 d = da;
}
