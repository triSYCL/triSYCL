#include <vector>
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  queue my_queue;
  const int size = 10;
  int data[size];
  const int groupsize = 2;
//////// Start slide
buffer<int> my_buffer(data, size);

command_group(my_queue, [&]()
{
	auto in_access = my_buffer.get_access<access::read>();
	auto out_access = my_buffer.get_access<access::write>();

	parallel_for_workgroup(nd_range<>(range<>(size), range<>(groupsize)),
                         kernel_lambda<class hierarchical>([=](group<> group)
	{
		parallel_for_workitem(group, [=](item<1> tile)
		{
			out_access[tile] = in_access[tile] * 2;
		});
	}));
});
//////// End slide
    return 0;
}
