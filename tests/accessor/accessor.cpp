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

    A[3]= 57;
    VERIFY_READ_WRITE_VALUE(A[3], 57);
    VERIFY_READ_WRITE_VALUE(A[id<1> { 18 }], 7);
    VERIFY_READ_WRITE_VALUE(A[item<1> (N, 2)], 75);
    VERIFY_READ_WRITE_VALUE(A[item<1> (N, 13, 0)], 57);
    VERIFY_READ_WRITE_VALUE(A[item<1> (N, 10, 2)], 5);
    VERIFY_READ_WRITE_VALUE(A[nd_item<1> (7, nd_range<1>{ N, 3 })], 15);

    VERIFY_READ_WRITE_VALUE((B[3][8]), 17);
    VERIFY_READ_WRITE_VALUE(B[id<2> (8, 7)], 87);
    VERIFY_READ_WRITE_VALUE(B[item<2> ({ N, M }, { 2, 3 })], 175);
    VERIFY_READ_WRITE_VALUE(B[item<2> ({ N, M }, { 12, 13 }, { 0, 0 })], 527);
    VERIFY_READ_WRITE_VALUE(B[item<2> ({ N, M }, { 19, 23 }, { 50, 40 })], 5);
    VERIFY_READ_WRITE_VALUE(B[nd_item<2> ({ 7, 5 },
                                          nd_range<2>{ { N, M },
                                              { 4, 4 } })], 9);

    // Check the pointer method
    VERIFY_COND(*(A.get_pointer() + 3) == 57);

  return 0;
}
