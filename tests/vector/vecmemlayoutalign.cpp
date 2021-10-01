/* RUN: %{execute}%s

   Testing vector element alignment, size and layout to see that it meets:
   SYCL 1.2.1 Section 4.10.2.6 Memory layout and alignment
*/

#include <CL/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;

template <typename T>
struct Matrix4x4 {
  cl::sycl::vec<T, 4> data[4];
};


template <typename T>
void TestVecType() {
  Matrix4x4<T> Mt;

  // We don't actually care what these values are, just that they are consistent
  // and checkable. Its A, B, C and D in the ASCII table, 65,66,67,68 in decimal
  for (int i = 0; i < 4; ++i) {
    Mt.data[i] = {0x41, 0x42, 0x43, 0x44};
  }

  cl::sycl::vec<T, 1> v1;
  cl::sycl::vec<T, 2> v2;
  cl::sycl::vec<T, 3> v3;
  cl::sycl::vec<T, 4> v4;
  cl::sycl::vec<T, 8> v8;

  REQUIRE(sizeof(v1) == 1 * sizeof(T));
  REQUIRE(sizeof(v2) == 2 * sizeof(T));
  REQUIRE(sizeof(v3) == 4 * sizeof(T));
  REQUIRE(sizeof(v4) == 4 * sizeof(T));
  REQUIRE(sizeof(v8) == 8 * sizeof(T));

  REQUIRE(sizeof(v1) == v1.get_size());
  REQUIRE(sizeof(v2) == v2.get_size());
  REQUIRE(sizeof(v3) == v3.get_size());
  REQUIRE(sizeof(v4) == v4.get_size());
  REQUIRE(sizeof(v8) == v8.get_size());

  REQUIRE(sizeof(Mt.data[0][0]) == sizeof(T));
  REQUIRE(sizeof(Mt.data[0]) == v4.get_size());
  REQUIRE(sizeof(Mt) == v4.get_size() * 4);

  // is sequential and contiguous in memory
  for (int i = 0; i < 4; ++i)
    REQUIRE(static_cast<void*>(&v4[i]) ==
            static_cast<void*>(&v4[0] + i));

  // A magic trick! reinterpret_cast to a T* as the data should be contiguous
  // and sequential and thus usable as an array
  const T* data = reinterpret_cast<const T*>(&Mt);

  v4 = {0x41, 0x42, 0x43, 0x44};

  // Perhaps there is a better test than this... it's just checking that each
  // sycl::vec in the structure is contiguous to the previous one and that the
  // data is correct
  for (int i = 0, k = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j) {
      REQUIRE(Mt.data[i][j] == v4[j % 4]);

      REQUIRE(static_cast<void*>(&Mt.data[i][j]) ==
              static_cast<void*>(&Mt.data[0][0] + k));

      REQUIRE(Mt.data[i][j] == *(&Mt.data[0][0] + k));

      REQUIRE(data[k] == *(&Mt.data[0][0] + k));

      REQUIRE(data[k] == Mt.data[i][j]);

      ++k;
    }

  // Check alignment is still working, technically not required as there exists
  // an align as test in vecalign, but doesn't hurt to be thorough
  REQUIRE(std::alignment_of<decltype(v1)>::value == sizeof(T) * 1);
  REQUIRE(std::alignment_of<decltype(v2)>::value == sizeof(T) * 2);
  REQUIRE(std::alignment_of<decltype(v3)>::value == sizeof(T) * 4);
  REQUIRE(std::alignment_of<decltype(v4)>::value == sizeof(T) * 4);
  REQUIRE(std::alignment_of<decltype(v8)>::value == sizeof(T) * 8);
}

TEST_CASE("memory layoout", "[vector]") {
  TestVecType<char>();
  TestVecType<int>();
  TestVecType<float>();
  TestVecType<double>();
}
