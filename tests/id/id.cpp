/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1
   CHECK-NEXT: 5 7
   CHECK-NEXT: 3
   CHECK-NEXT: jj has changed
   CHECK-NEXT: jj via e = 5
   CHECK-NEXT: jj via e = 3
   CHECK-NEXT: cjj via e = 5
   CHECK-NEXT: cjj via e = 3
   CHECK-NEXT: 6 5
   CHECK-NEXT: 4 4
   CHECK-NEXT: 0 1
   CHECK-NEXT: 12 15
   CHECK-NEXT: 6 2
   CHECK-NEXT: 1 2 3
   CHECK-NEXT: 5 6
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;


int main() {
  id<> i;
  id<3> i3;
  id<> j { 1 };
  i = j;
  id<2> ii;
  id<2> jj { 5, 7 };
  ii = jj;
  jj[1] = 3;
  i3 = { 1, 2, 3 };

  std::cout << std::endl << "Result:" << std::endl;
  std::cout << i.get(0) << std::endl;
  std::cout << ii.get(0) << " " << ii.get(1) << std::endl;
  std::cout << jj[1] << std::endl;

  // Test that id<> implement comparable
  if (jj != make_id({ 5, 7 }))
    std::cout << "jj has changed" << std::endl;

  // Try some STL interoperability:

  // Check that the for range loop works
  for (auto const & e: jj)
    std::cout << "jj via e = " << e << std::endl;

  std::vector<float> cjj(std::begin(jj), std::end(jj));

  for (auto e: cjj)
    std::cout << "cjj via e = " << e << std::endl;

  // Test arithmetics on id<>

  jj += make_id({ 1, 2 });
  jj.display();
  (jj - make_id({ 2, 1 })).display();
  (jj % make_id({ 2, 2 })).display();
  (jj * make_id({ 2, 3 })).display();
  (jj /= make_id(1, 2)).display();

  range<3> r = { 1, 2, 3 };
  id<3> ir { r };
  ir.display();

  item<2> it { { 1, 2 }, { 5, 6 }, { 9, 10 } };
  id<2> iditem { it };
  iditem.display();

  return 0;
}
