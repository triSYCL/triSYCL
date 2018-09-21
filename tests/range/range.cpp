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
  range<> t { 1 };
  range<> r(t);

  std::cout << std::endl << "Result:" << std::endl;
  std::cout << r.get(0) << " ";
  std::cout << std::endl;

  range<> a = 3;
  std::cout << "a = " << a[0] << std::endl;
  range<1> b = { 4 };
  std::cout << "b = " << b[0] << std::endl;
  b /= make_range(2);
  std::cout << "b = " << b[0] << std::endl;
  range<2> c = { 5, 6 };
  std::cout << "c = " << c[0] << ',' << c[1] << std::endl;
  range<3> d = { 7, 8, 9 };
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;
  auto div = d/d;
  div.display();

  // Test modulo operation
  d %= make_range({2, 3, 4});
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;

  d += make_range(1, 2, 3);
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;

  g(43);
  g(2014);
  g({ 1, 128 });

  g({ 11, 54, 68 });


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
  return 0;
}
