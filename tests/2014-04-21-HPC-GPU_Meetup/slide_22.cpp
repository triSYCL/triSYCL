/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: data[5] = 10, should be 10
*/
#include <vector>
#include <algorithm>
#include <iostream>
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  queue my_queue;
  const int size = 10;
  int data[size];
  // Initialize the data with the index value
  std::iota(std::begin(data), std::end(data), 0);
  const int groupsize = 2;
  {
//////// Start slide
buffer<int> my_buffer(data, size);

my_queue.submit([&](handler &cgh)
{
  auto in_access = my_buffer.get_access<access::read>(cgh);
  auto out_access = my_buffer.get_access<access::write>(cgh);

  cgh.parallel_for_work_group<class hierarchical>(nd_range<>(range<>(size),
                                                             range<>(groupsize)),
                                                  [=](group<> group) {
                                                    parallel_for_work_item(group, [=](nd_item<1> tile) {
                                                        out_access[tile] = in_access[tile] * 2;
                                                      });
                                                  });
 });
  //////// End slide
  }
  std::cout << "data[5] = " << data[5] << ", should be 10" << std::endl;
  return 0;
}
