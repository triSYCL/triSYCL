/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1
   CHECK-NEXT: a = 3
   CHECK-NEXT: b = 4
   CHECK-NEXT: b = 2
   CHECK-NEXT: c = 5,6
   CHECK-NEXT: d = 7,8,9
   CHECK-NEXT: 1 1 1
   CHECK-NEXT: d = 1,2,1
   CHECK-NEXT: d = 2,4,4
   CHECK-NEXT: Range of dims 1
   CHECK-NEXT: Range of dims 1
   CHECK-NEXT: Range of dims 2
   CHECK-NEXT: Range of dims 3
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 2 5 6
   CHECK-NEXT: 1 4 16
   CHECK-NEXT: 1 2 4
   CHECK-NEXT: 0 2 4
   CHECK-NEXT: 1 3 5
   CHECK-NEXT: 1 1 1
   CHECK-NEXT: 0
   CHECK-NEXT: 1
   CHECK-NEXT: 0
   CHECK-NEXT: 1
   CHECK_NEXT: 0 0 1
   CHECK_NEXT: 0 1 1
   CHECK_NEXT: 1 0 0
   CHECK_NEXT: 1 1 0
   CHECK_NEXT: 7 8 9
   CHECK_NEXT: 6 6 6
   CHECK_NEXT: 0 1 2
   CHECK_NEXT: 1 2 3
   CHECK_NEXT: 1 2 3
   CHECK_NEXT: 0 1 2
   CHECK_NEXT: 1 2 3
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

// To test the inference of the range rank
template<int N> void f(const range<N>& r) {
  std::cout << "Range of dims " << N << std::endl;
}

int main() {
  range<> t { 1 };
  range<> r(t);
  // CTAD
  range u { r };

  std::cout << std::endl << "Result:" << std::endl;
  std::cout << u.get(0) << " ";
  std::cout << std::endl;

  range<> a = 3;
  std::cout << "a = " << a[0] << std::endl;
  range<1> b = { 4 };
  std::cout << "b = " << b[0] << std::endl;
  b /= { 2 };
  std::cout << "b = " << b[0] << std::endl;
  range<2> c = { 5, 6 };
  std::cout << "c = " << c[0] << ',' << c[1] << std::endl;
  range<3> d = { 7, 8, 9 };
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;
  auto div = d/d;
  div.display();

  // Test modulo operation
  d %= { 2, 3, 4 };
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;

  d += { 1, 2, 3 };
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;

  f(43);
  f(2014);
  f({ 1, 128 });
  f({ 11, 54, 68 });


  // Try some conversions
  char array[3] = { 1, 2, 3 };
  // Use some explicit conversion
  range<3> r_array { array };
  r_array.display();
  // Conversion from an id<>
  r_array = id<3>(2,5,6);
  r_array.display();

  range<3> r1 = { 1, 2, 4 };
  r1 <<= { 0, 1, 2 };
  r1.display();
  (r1 >>= { 0, 1, 2 }).display();

  range<3> r2 = { 0, 3, 5 };
  auto r3 = r1 & r2;
  r3.display();
  r3 = r1 | r2;
  r3.display();
  (r1 ^ r2).display();
  std::cout << (r1!= r1) << std::endl;
  std::cout << (r1 == r1) << std::endl;
  std::cout << (r1 == r2) << std::endl;
  std::cout << (r1 != r2) << std::endl;

  r2[1] = 2;
  auto r4 = r1 < r2;
  r4.display();
  r4 = r1 <= r2;
  r4.display();
  r4 = r1 > r2;
  r4.display();
  r4 = r1 >= r2;
  r4.display();

  {
    range<3> r1 { 7, 8, 9 };
    (+r1).display();
    range<3> r2 { 1, 2, 3 };
    auto r3 = r1 + (-r2);
    r3.display();
    (--r2).display();
    (++r2).display();
    (r2--).display();
    (r2++).display();
    r2.display();
  }

  return 0;
}
