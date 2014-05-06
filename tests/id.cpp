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

  return 0;
}
