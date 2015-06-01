/* RUN: %{execute}%s | %{filecheck} %s
   CHECK: Group id = 0
   CHECK: Local id = 0 (global id = 0)
   CHECK: Local id = 1 (global id = 1)
   CHECK: Group id = 1
   CHECK: Local id = 0 (global id = 2)
   CHECK: Local id = 1 (global id = 3)
   CHECK: Group id = 2
   CHECK: Local id = 0 (global id = 4)
   CHECK: Local id = 1 (global id = 5)
   CHECK: Group id = 3
   CHECK: Local id = 0 (global id = 6)
   CHECK: Local id = 1 (global id = 7)
   CHECK: Group id = 4
   CHECK: Local id = 0 (global id = 8)
   CHECK: Local id = 1 (global id = 9)
*/
#include <vector>
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  queue my_queue;
  const int size = 10;
  std::vector<int> data(size);
  const int groupsize = 2;
/* Put &data[0] instead of data.data() because it is not obvious in the
   excerpt below it is a vector */
//////// Start slide
buffer<int> input(&data[0], size);
buffer<int> output(size);
my_queue.submit([&](handler &cgh)
{
	auto in_access = input.get_access<access::read>(cgh);
	auto out_access = output.get_access<access::write>(cgh);

	cgh.parallel_for_work_group<class hierarchical>(nd_range<>(range<>(size),
                                                             range<>(groupsize)),
                                                  [=](group<> group)
	{
        std::cout << "Group id = " << group.get(0) << std::endl;

		parallel_for_work_item(group, [=](item<1> tile)
		{
      size_t localID = tile.get(0);
      size_t groupID = group.get(0);
      size_t groupRange = group.get_local_range(0);
      size_t globalIDGroupStart = groupID*groupRange;
      size_t globalID = globalIDGroupStart + localID;
      std::cout << "Local id = " << localID
          << " (global id = " << (globalID) << ")" << std::endl;
			out_access[globalID] = in_access[globalID] * 2;
		});
	});
});
//////// End slide
    return 0;
}
