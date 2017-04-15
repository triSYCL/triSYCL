/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: 10
   CHECK: 5
   CHECK: 0
   CHECK: 10
   CHECK: 5
   CHECK: 0
   CHECK: 7
   CHECK: 7
   CHECK: 7
   CHECK: 2
   CHECK: 2
   CHECK: 2
   CHECK: 1
   CHECK: 1
   CHECK: 1
   CHECK: 2
   CHECK: 2
   CHECK: 100 10
   CHECK: 2 5
   CHECK: 0 0
   CHECK: 100 10
   CHECK: 2 5
   CHECK: 0 0
   CHECK: 99 9
   CHECK: 99 9
   CHECK: 999
   CHECK: 1 4
   CHECK: 1 4
   CHECK: 9
   CHECK: 49 1
   CHECK: 49 1
   CHECK: 99
   CHECK: 50 2
   CHECK: 50 2
   CHECK: 10 9 12
   CHECK: 2 3 6
   CHECK: 1 2 3
   CHECK: 10 9 12
   CHECK: 2 3 6
   CHECK: 1 2 3
   CHECK: 9 8 11
   CHECK: 9 8 11
   CHECK: 788
   CHECK: 0 0 2
   CHECK: 0 0 2
   CHECK: 12
   CHECK: 4 2 1
   CHECK: 4 2 1
   CHECK: 29
   CHECK: 5 3 2
   CHECK: 5 3 2
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

#define DISPLAY_ELEMENTS(method)                                \
  do {                                                          \
    for (int i = 0; i != decltype(ndi)::dimensionality; ++i)    \
      std::cout << ndi.method(i) << ' ';                        \
    std::cout << std::endl;                                     \
  } while (0)

auto display_test = [](auto ndi) {
  ndi.get_nd_range().display();
  ndi.get_global_range().display();
  ndi.get_local_range().display();
  ndi.get_offset().display();
  ndi.get_global().display();
  DISPLAY_ELEMENTS(get_global);
  std::cout << ndi.get_global_linear_id() << std::endl;
  ndi.get_local().display();
  DISPLAY_ELEMENTS(get_local);
  std::cout << ndi.get_local_linear_id() << std::endl;
  ndi.get_group().display();
  DISPLAY_ELEMENTS(get_group);
  std::cout << ndi.get_group_linear_id() << std::endl;
  ndi.get_num_groups().display();
  DISPLAY_ELEMENTS(get_num_groups);
};

int main() {
  nd_item<> i { 7, { 10, 5 } };
  display_test(i);

  nd_item<2> i2 { { 99, 9 }, { { 100, 10 }, { 2, 5 } } };
  display_test(i2);

  nd_item<3> i3 { {9, 8, 11 }, { { 10, 9, 12 }, { 2, 3, 6 }, { 1, 2, 3 } } };
  display_test(i3);

  return 0;
}
