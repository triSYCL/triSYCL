/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1
   CHECK-NEXT: a = 3
   CHECK-NEXT: b = 4
   CHECK-NEXT: c = 5,6
   CHECK-NEXT: d = 7,8,9
   CHECK-NEXT: Range of dims 1
   CHECK-NEXT: Range of dims 1
   CHECK-NEXT: Range of dims 2
   CHECK-NEXT: Range of dims 3
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

  range<1> a = 3;
  std::cout << "a = " << a[0] << std::endl;
  range<1> b = { 4 };
  std::cout << "b = " << b[0] << std::endl;
  range<2> c = { 5, 6 };
  std::cout << "c = " << c[0] << ',' << c[1] << std::endl;
  range<3> d = { 7, 8, 9 };
  std::cout << "d = " << d[0] << ',' << d[1] <<  ',' << d[2] << std::endl;

  g(43);
  g({ 2014 });
  g({ 1, 128 });
  g({ 11, 54, 68 });
  return 0;
}
