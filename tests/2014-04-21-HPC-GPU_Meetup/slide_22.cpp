buffer<int> my_buffer(data, 10);
 
command_group(my_queue, [&]()
{
	auto in_access = my_buffer.access<cl::sycl::access:read_only>();
	auto out_access = my_buffer.access<cl::sycl::access:write_only>();

	parallel_for_workgroup(nd_range(range(size), range(groupsize)), 
		lambda<class hierarchical>([=](group_id group)
	{
		parallel_for_workitem(group, [=](tile_id tile)
		{
			out_access[tile] = in_access[tile] * 2;
		});
	}));
});
