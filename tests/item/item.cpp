/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 1
   CHECK-NEXT: 2
   CHECK-NEXT: 3
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 2 3 6
   CHECK-NEXT: 10 11 12
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;


int main() {
  item<> i { 1, 2, 3 };
  i.get_range().display();
  i.get_global_id().display();
  i.get_offset().display();

  item<3> i3 { { 1, 2, 3 }, { 2, 3, 6 }, { 10, 11, 12 } };
  i3.get_range().display();
  i3.get_global_id().display();
  i3.get_offset().display();

  return 0;
}
