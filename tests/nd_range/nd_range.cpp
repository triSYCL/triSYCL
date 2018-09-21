/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 6 4 8
   CHECK-NEXT: 3 2 4
   CHECK-NEXT: 0 0 0
   CHECK-NEXT: 6 4 8
   CHECK-NEXT: 3 2 4
   CHECK-NEXT: 0 0 0
   CHECK-NEXT: 2 2 2
   CHECK-NEXT: 2 2 2
   CHECK-NEXT: 4
   CHECK-NEXT: 2
   CHECK-NEXT: 3
   CHECK-NEXT: 10 20
   CHECK-NEXT: 2 5
   CHECK-NEXT: 0 0
   CHECK-NEXT: 1
   CHECK-NEXT: 6 4
   CHECK-NEXT: 6 5 7
   CHECK-NEXT: 1
   CHECK-NEXT: 0
   CHECK-NEXT: 0
   CHECK-NEXT: 1
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

// To test the inference of the range rank
template<int N> void f(range<N> r) {
  std::cout << "Range of dims " << N << std::endl;
}

/* To force the implicit conversion to the right range<>

   Unfortunately this does not work with templates, even with explicit
   instanciation for these 3 values... */
void g(range<1> r) { f(r); }
void g(range<2> r) { f(r); }
void g(range<3> r) { f(r); }



int main() {
  nd_range<3> ndr3 { range<3>{ 6, 4, 8 }, range<3>{ 3, 2, 4} };
  ndr3.display();

  ndr3.get_global_range().display();
  ndr3.get_local_range().display();
  ndr3.get_offset().display();
  ndr3.get_group_range().display();
  range<3> ndr4 = ndr3.get_group_range();
  ndr4.display();

  nd_range<> ndr1 { 4, 2, 3 };
  ndr1.display();
  nd_range<2> ndr2 { { 10, 20 }, { 2, 5 } };
  ndr2.display();

  // Test now for incomplete work-groups
  nd_range<> ndri1 { 1, 2, 3 };
  ndri1.get_group_range().display();

  nd_range<2> ndri2 { { 11, 25 }, { 2, 7 } };
  ndri2.get_group_range().display();

  nd_range<3> ndri3 { { 11, 25, 48 }, { 2, 5, 7 } };
  ndri3.get_group_range().display();

  nd_range<> ndric { 1, 2, 3 };
  std::cout << (ndri1 == ndric) << std::endl;
  std::cout << (ndri1 != ndric) << std::endl;

  nd_range<> ndrinc { 1, 2, 5 };
  std::cout << (ndri1 == ndrinc) << std::endl;
  std::cout << (ndri1 != ndrinc) << std::endl;
  return 0;
}
