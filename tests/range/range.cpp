/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Result:
   CHECK-NEXT: 1
*/
#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;


int main() {
  range<> t { 1 };
  range<> r(t);

  std::cout << std::endl << "Result:" << std::endl;
  std::cout << r.get(0) << " ";
  std::cout << std::endl;

  return 0;
}
