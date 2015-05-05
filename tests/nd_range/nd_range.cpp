/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 6 4 8
   CHECK-NEXT: 3 2 4
   CHECK-NEXT: 0 0 0
   CHECK-NEXT: 6 4 8
   CHECK-NEXT: 3 2 4
   CHECK-NEXT: 0 0 0
   CHECK-NEXT: 2 2 2
   CHECK-NEXT: 2 2 2
   CHECK-NEXT: 1
   CHECK-NEXT: 2
   CHECK-NEXT: 3
   CHECK-NEXT: 10 20
   CHECK-NEXT: 2 5
   CHECK-NEXT: 0 0
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

// To test the inference of the range rank
template<std::size_t N> void f(range<N> r) {
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

  ndr3.get_global().display();
  ndr3.get_local().display();
  ndr3.get_offset().display();
  ndr3.get_group().display();
  range<3> ndr4 = ndr3.get_group();
  ndr4.display();

  nd_range<> ndr1 { 1, 2, 3 };
  ndr1.display();
  nd_range<2> ndr2 { { 10, 20 }, { 2, 5 } };
  ndr2.display();
  return 0;
}
