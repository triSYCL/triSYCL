#include <vector>
#include <CL/sycl.hpp>


int main() 
{
  using namespace cl::sycl;

  queue my_queue;
  const int size = 12;
  std::vector<int> data(size);
  for (int i = 0; i < size; ++i)
  {
    data[i] = size - i;
  }
  const int groupsize = 4;
  // Sub-group size is 2, chosen by runtime

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
        size_t subGroupID = subGroup;
        size_t groupID = group.get(0);
        size_t groupRange = group.get_local_range(0);
        size_t globalIDGroupStart = groupID*groupRange;
        size_t globalIDSubGroupStart = globalIDGroupStart + subGroupID*subGroup.get_local_range().get(0);

        std::cout << "Subgroup id = " << subGroup.get() << std::endl;
        
        auto idx = subGroup.get_index_vector();
        auto vec3 = subGroup.make_sgvec(static_cast<size_t>(3));
        auto vec5 = subGroup.make_sgvec(static_cast<size_t>(5));
        auto vec8 = vec3 + vec5 + idx;
        
        auto gatherVec = subGroup.make_sgvec<int*>(&in_access[0]);
        gatherVec = gatherVec + globalIDSubGroupStart;

        // Add inverted index to gather vector
        parallel_for_work_item(subGroup, [=, &gatherVec](item<1> tile)
        {
          gatherVec.get(tile) = gatherVec.get(tile) + (idx.size() - (idx.get(tile) + 1));
        });

        auto loadData = subGroup.gather_vec(gatherVec);

        auto outputVec = subGroup.make_sgvec<int>(0);
        parallel_for_work_item(subGroup, [=, &outputVec](item<1> tile)
        {
          size_t localID = tile.get(0);
          size_t globalID = globalIDSubGroupStart + localID;
          
          // Interface with vector operations
          size_t vecVal = vec8.get(tile);
          int shuffleInput = loadData.get(tile);

          std::cout
            << "vecVal = " << vecVal
            << " shuffleInput = " << shuffleInput
            << " (local id = " << localID << ")"
            << " (sub-group id = " << (subGroupID) << ")"
            << " (global id = " << (globalID) << ")" << std::endl;

          outputVec.get(tile) = static_cast<int>(vecVal + shuffleInput);
        });

        subGroup.store_vec(&out_access[0], outputVec);
      });
    });
  });

  return 0;
}
