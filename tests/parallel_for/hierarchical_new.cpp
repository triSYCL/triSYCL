/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Group id = 0
   CHECK: Local id = 0 (global id = 0)
   CHECK: Local id = 1 (global id = 1)
   CHECK: Global id = 0
   CHECK: Global id = 1
   CHECK: Group id = 1
   CHECK: Local id = 0 (global id = 2)
   CHECK: Local id = 1 (global id = 3)
   CHECK: Global id = 2
   CHECK: Global id = 3
   CHECK: Group id = 2
   CHECK: Local id = 0 (global id = 4)
   CHECK: Local id = 1 (global id = 5)
   CHECK: Global id = 4
   CHECK: Global id = 5
   CHECK: Group id = 3
   CHECK: Local id = 0 (global id = 6)
   CHECK: Local id = 1 (global id = 7)
   CHECK: Global id = 6
   CHECK: Global id = 7
   CHECK: Group id = 4
   CHECK: Local id = 0 (global id = 8)
   CHECK: Local id = 1 (global id = 9)
   CHECK: Global id = 8
   CHECK: Global id = 9
*/

/* The OpenMP based barrier use nested parallelism that makes order of
   execution in parallel_for_workitem non deterministic so disable it on
   this test
*/
#define TRISYCL_NO_BARRIER


#include <vector>
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  queue my_queue;
  const int size = 10;
  std::vector<int> data(size);
  constexpr int groupsize = 2;
/* Put &data[0] instead of data.data() because it is not obvious in the
   excerpt below it is a vector */
//////// Start slide
buffer<int> input(&data[0], size);
buffer<int> output(size);
my_queue.submit([&](handler &cgh) {
  auto in_access = input.get_access<access::mode::read>(cgh);
  auto out_access = output.get_access<access::mode::write>(cgh);

  cgh.parallel_for_work_group<class hierarchical>(nd_range<>(range<>(size),
                                                             range<>(groupsize)),
                                                  [=](group<> group) {
    std::cout << "Group id = " << group.get_id(0) << std::endl;

    group.parallel_for_work_item([=](h_item<1> tile) {
      std::cout << "Local id = " << tile.get_local(0)
                << " (global id = " << tile.get_global(0) << ")" << std::endl;
      out_access[tile.get_global()] = in_access[tile.get_global()] * 2;
    });

    group.parallel_for_work_item([=](h_item<1> tile) {
      std::cout << "Global id = " << tile.get_global(0) << std::endl;
      out_access[tile.get_global()] = in_access[tile.get_global()] * 2;
    });
  });
});
//////// End slide
  /* We must wait for for the queue to finish as none of buffer's destruction
     is blocking.
   */
  my_queue.wait();
  return 0;
}
