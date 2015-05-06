/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 7
   CHECK-NEXT: 3
   CHECK-NEXT: 2
   CHECK-NEXT: 1
   CHECK-NEXT: 10 11 12
   CHECK-NEXT: 2 3 6
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 341
   CHECK-NEXT: 100 10
   CHECK-NEXT: 2 3
   CHECK-NEXT: 0 0
   CHECK-NEXT: 302
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;


int main() {
  item<> i { 7, 3, 2 };
  i.get_range().display();
  i.get_global_id().display();
  i.get_offset().display();
  std::cout << i.get_linear_id() << std::endl;
  item<3> i3 { { 10, 11, 12 }, { 2, 3, 6 }, { 1, 2, 3 } };
  i3.get_range().display();
  i3.get_global_id().display();
  i3.get_offset().display();
  std::cout << i3.get_linear_id() << std::endl;

  item<2> i2 { { 100, 10 }, { 2, 3 } };
  i2.get_range().display();
  i2.get_global_id().display();
  i2.get_offset().display();
  std::cout << i2.get_linear_id() << std::endl;

  return 0;
}
