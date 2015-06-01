#include <vector>
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  queue my_queue;
  const int size = 12;
  std::vector<int> data(size);
  const int groupsize = 4;
  // Sub-group size is 2, chosen by runtime

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

      parallel_for_sub_group(group, [=](subgroup subGroup)
      {
        std::cout << "Subgroup id = " << subGroup.get() << std::endl;
        
        // TODO: continue work on sgvec code
        auto idx = subGroup.get_index_vector();
        auto vec3 = subGroup.make_sgvec(static_cast<size_t>(3));
        auto vec5 = subGroup.make_sgvec(static_cast<size_t>(5));
        auto vec8 = vec3 + vec5 + idx;
        
        parallel_for_work_item(subGroup, [=](item<1> tile)
        {
          size_t localID = tile.get(0);
          size_t subGroupID = subGroup;
          size_t groupID = group.get(0);
          size_t groupRange = group.get_local_range(0);
          size_t globalIDGroupStart = groupID*groupRange;
          size_t globalIDSubGroupStart = globalIDGroupStart + subGroupID*subGroup.get_local_range().get(0);
          size_t globalID = globalIDSubGroupStart + localID;
          
          // Interface with vector operations
          size_t vecVal = vec8.get(tile);

          std::cout 
            << "Local id = " << localID
            << " vecVal = " << vecVal
            << " (sub-group id = " << (subGroupID) << ")"
            << " (global id = " << (globalID) << ")" << std::endl;

          out_access[globalID] = static_cast<int>(in_access[globalID] * 2 + vecVal);
        });
      });
    });
  });

  return 0;
}
