/* RUN: %{execute}%s
*/
#include <CL/sycl.hpp>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include "test-helpers.hpp"

using namespace cl::sycl;

// Size of the buffers
constexpr size_t N = 20;
constexpr size_t M = 30;
constexpr size_t P = 40;

#define TEST_TYPE(accessor_var, type_member, type)                      \
  static_assert(std::is_same<type,                                      \
                decltype(accessor_var)::type_member>::value,            \
                #type_member " is an " #type)

int main() {
    // Create read-write buffers for each rank
    cl::sycl::buffer<int, 1> a(N);
    cl::sycl::buffer<int, 2> b({ N, M });
    cl::sycl::buffer<int, 3> c({ N, M, P });

    // Test from the host
    auto A = a.get_access<cl::sycl::access::mode::read_write>();
    auto B = b.get_access<cl::sycl::access::mode::read_write>();
    auto C = c.get_access<cl::sycl::access::mode::read_write>();

    // Test the STL-like accessor member types
    TEST_TYPE(A, value_type, int);
    TEST_TYPE(A, pointer, int *);
    TEST_TYPE(B, const_pointer, const int *);
    TEST_TYPE(B, reference, int &);
    TEST_TYPE(C, const_reference, const int &);

    VERIFY_READ_WRITE_VALUE(A[3], 57);
    VERIFY_READ_WRITE_VALUE(A[id<1> { 18 }], 7);
    VERIFY_READ_WRITE_VALUE(A[item<1> (N, 2)], 75);
    VERIFY_READ_WRITE_VALUE(A[nd_item<1> (7, nd_range<1>{ N, 3 })], 15);

    VERIFY_READ_WRITE_VALUE((B[3][8]), 17);
    VERIFY_READ_WRITE_VALUE(B[id<2> (8, 7)], 87);
    VERIFY_READ_WRITE_VALUE(B[item<2> ({ N, M }, { 2, 3 })], 175);
    VERIFY_READ_WRITE_VALUE(B[nd_item<2> ({ 7, 5 },
                                          nd_range<2>{ { N, M },
                                              { 4, 4 } })], 9);
    VERIFY_READ_WRITE_VALUE((C[3][8][5]), 17);
    VERIFY_READ_WRITE_VALUE(C[id<3> (8, 7, 3)], 87);
    VERIFY_READ_WRITE_VALUE(C[item<3> ({ N, M, P }, { 2, 3, 5 })], 175);
    VERIFY_READ_WRITE_VALUE(C[nd_item<3> ({ 7, 5, 3 },
                                          nd_range<3>{ { N, M, P },
                                              { 11, 5, 3 } })], 7);

    // Check the pointer method
    VERIFY_COND(*(A.get_pointer() + 3) == 57);

  return 0;
}
