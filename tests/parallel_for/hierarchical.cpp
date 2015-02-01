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

command_group(my_queue, [&]()
{
	auto in_access = input.get_access<access::read>();
	auto out_access = output.get_access<access::write>();

	parallel_for_workgroup<class hierarchical>(nd_range<>(range<>(size),
                                                        range<>(groupsize)),
                                             [=](group<> group)
	{
    std::cout << "Group id = " << group.get(0) << std::endl;

		parallel_for_workitem(group, [=](nd_item<1> tile)
		{
      std::cout << "Local id = " << tile.get_local_id(0)
                << " (global id = " << tile.get_global_id(0) << ")" << std::endl;
			out_access[tile] = in_access[tile] * 2;
		});
	});
});
//////// End slide
    return 0;
}
