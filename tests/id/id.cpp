/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1
   CHECK-NEXT: 5 7
   CHECK-NEXT: 3
   CHECK: as_an_int = 42
   CHECK-NEXT: jj via e = 5
   CHECK-NEXT: jj via e = 3
   CHECK-NEXT: cjj via e = 5
   CHECK-NEXT: cjj via e = 3
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;


int main() {
  id<> i;
  id<> k(i);
  id<> j { 1 };
  i = j;
  id<2> ii;
  id<2> jj { 5, 7 };
  ii = jj;
  jj[1] = 3;

  std::cout << std::endl << "Result:" << std::endl;
  std::cout << i.get(0) << std::endl;
  std::cout << ii.get(0) << " " << ii.get(1) << std::endl;
  std::cout << jj[1] << std::endl;
  std::cout << std::endl;

  id <> as_an_int;
  as_an_int = 42;
  std::cout << "as_an_int = " << as_an_int << std::endl;

  // Try some STL interoperability:

  // Check the for range loop works
  for(auto const & e: jj)
    std::cout << "jj via e = " << e << std::endl;

  std::vector<float> cjj(std::begin(jj), std::end(jj));

  for(auto e: cjj)
    std::cout << "cjj via e = " << e << std::endl;
  return 0;
}
