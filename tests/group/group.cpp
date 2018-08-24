/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 10
   CHECK: 2
   CHECK: 0
   CHECK: 5
   CHECK: 5
   CHECK: 10
   CHECK: 10
   CHECK: 2
   CHECK: 2
   CHECK: 0
   CHECK: 0
   CHECK: 3
   CHECK: 3
   CHECK: 3
   CHECK: 100 10
   CHECK: 2 5
   CHECK: 0 0
   CHECK: 50 2
   CHECK: 50 2
   CHECK: 100 10
   CHECK: 100 10
   CHECK: 2 5
   CHECK: 2 5
   CHECK: 0 0
   CHECK: 0 0
   CHECK: 49 1
   CHECK: 49 1
   CHECK: 99
   CHECK: 10 9 12
   CHECK: 2 3 6
   CHECK: 1 2 3
   CHECK: 5 3 2
   CHECK: 5 3 2
   CHECK: 10 9 12
   CHECK: 10 9 12
   CHECK: 2 3 6
   CHECK: 2 3 6
   CHECK: 1 2 3
   CHECK: 1 2 3
   CHECK: 4 2 1
   CHECK: 4 2 1
   CHECK: 29
   CHECK: 1
   CHECK: 0
   CHECK: 0
   CHECK: 1
   CHECK: 0
   CHECK: 1
*/

#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

#define DISPLAY_ELEMENTS(method)                                \
  do {                                                          \
    for (size_t i = 0; i != decltype(ndi)::dimensionality; ++i) \
      std::cout << ndi.method(i) << ' ';                        \
    std::cout << std::endl;                                     \
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
  ndi.get_id().display();
  DISPLAY_ELEMENTS(get_id);
  std::cout << ndi.get_linear_id() << std::endl;
};

int main() {
  group<> g { 3, { 10, 2 } };
  display_test(g);

  group<2> i2 { { 49, 1 }, { { 100, 10 }, { 2, 5 } } };
  display_test(i2);

  group<3> i3 { {4, 2, 1 }, { { 10, 9, 12 }, { 2, 3, 6 }, { 1, 2, 3 } } };
  display_test(i3);

  group<> g2 { 3, { 10, 2 }};
  std::cout << (g == g2) << std::endl;
  std::cout << (g != g2) << std::endl;

  group<> g3 { 3, { 11, 2 }};
  std::cout << (g == g3) << std::endl;
  std::cout << (g != g3) << std::endl;

  group<> g4 { 4, { 10, 2 }};
  std::cout << (g == g4) << std::endl;
  std::cout << (g != g4) << std::endl;
  return 0;
}
