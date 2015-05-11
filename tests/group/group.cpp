/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 10
   CHECK-NEXT: 2
   CHECK-NEXT: 0
   CHECK-NEXT: 5
   CHECK-NEXT: 5
   CHECK-NEXT: 10
   CHECK-NEXT: 10
   CHECK-NEXT: 2
   CHECK-NEXT: 2
   CHECK-NEXT: 0
   CHECK-NEXT: 0
   CHECK-NEXT: 3
   CHECK-NEXT: 3
   CHECK-NEXT: 3
   CHECK-NEXT: 100 10
   CHECK-NEXT: 2 5
   CHECK-NEXT: 0 0
   CHECK-NEXT: 50 2
   CHECK-NEXT: 50 2
   CHECK-NEXT: 100 10
   CHECK-NEXT: 100 10
   CHECK-NEXT: 2 5
   CHECK-NEXT: 2 5
   CHECK-NEXT: 0 0
   CHECK-NEXT: 0 0
   CHECK-NEXT: 49 1
   CHECK-NEXT: 49 1
   CHECK-NEXT: 99
   CHECK-NEXT: 10 9 12
   CHECK-NEXT: 2 3 6
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 5 3 2
   CHECK-NEXT: 5 3 2
   CHECK-NEXT: 10 9 12
   CHECK-NEXT: 10 9 12
   CHECK-NEXT: 2 3 6
   CHECK-NEXT: 2 3 6
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 4 2 1
   CHECK-NEXT: 4 2 1
   CHECK-NEXT: 29
*/

#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

#define DISPLAY_ELEMENTS(method)                            \
  do {                                                      \
    for (int i = 0; i < decltype(ndi)::dimensionality; ++i) \
      std::cout << ndi.method(i) << ' ';                    \
    std::cout << std::endl;                                 \
  } while (0)

auto display_test = [](auto ndi) {
  ndi.get_nd_range().display();
  ndi.get_group_range().display();
  DISPLAY_ELEMENTS(get_group_range);
  ndi.get_global_range().display();
  DISPLAY_ELEMENTS(get_global_range);
  ndi.get_local_range().display();
  DISPLAY_ELEMENTS(get_local_range);
  ndi.get_offset().display();
  DISPLAY_ELEMENTS(get_offset);
  ndi.get().display();
  DISPLAY_ELEMENTS(get);
  std::cout << ndi.get_linear() << std::endl;
};

int main() {
  group<> g { 3, { 10, 2 } };
  display_test(g);

  group<2> i2 { { 49, 1 }, { { 100, 10 }, { 2, 5 } } };
  display_test(i2);

  group<3> i3 { {4, 2, 1 }, { { 10, 9, 12 }, { 2, 3, 6 }, { 1, 2, 3 } } };
  display_test(i3);

  return 0;
}
