#include <vector>
#include <CL/sycl.hpp>

using namespace cl::sycl;

int main() {
  queue my_queue;
  constexpr int size = 12;

  
  //constexpr int radius = 5;
  //constexpr int groupsize = 2;
  // NB: Current loops use group directly so blocksize must equal groupsize
  //const int blocksize = groupsize;
  // constexpr capture seems broken in VS2015 so add unfortunate preprocessor macros.
#define radius 5
#define groupsize 6
#define blocksize groupsize
  
  std::vector<float> data(size);
  for (int i = 0; i < size; ++i) {
    data[i] = static_cast<float>(i);
  }
  std::vector<float> filter(radius*2+1, 1.f);
    
  buffer<float> inputB(data.data(), size);
  buffer<float> filterB(filter.data(), size);
  buffer<float> outputB(size);

  my_queue.submit([&](handler &cgh)
  {
    auto in_access = inputB.get_access<access::read>(cgh);
    auto filter_access = filterB.get_access<access::read>(cgh);
    auto out_access = outputB.get_access<access::write>(cgh);

    cgh.parallel_for_work_group<class convolution>(nd_range<>(range<>(size),
                                                              range<>(groupsize)),
                                                    [=](group<> group)
    {
      std::cout << "Group id = " << group.get(0) << std::endl;

      // Local versions of data to simplify algorithm
      float filterLocal[2*radius+1];
      float localData[blocksize + 2 * radius];
      float convolutionResult[blocksize];

      // Load filter
      range<1> filterRange{ 2 * radius + 1 };
      // Question: Should this really parameterise with an id?
      // That would make private memory no longer work, but conceptually
      // it would be cleaner
      parallel_for_work_item(group, filterRange, [&](item<1> tile)
      {
        filterLocal[tile.get()] = filter_access[tile.get()];
      });

      // Load convolution data
      range<1> inputRange{ blocksize + 2 * radius };
      parallel_for_work_item(group, inputRange, [&](item<1> tile)
      {
        // From the input data offset by the beginning of this group's block
        // and add the iteration
        float val = 0.f;
        // Compute read address and bound it to the size of the input
        int readAddress = static_cast<int>(group.get() * id<1>{blocksize} + id<1>{tile}) - radius;
        if (readAddress >= 0 && readAddress < size) {
          val = in_access[readAddress];
        }
        localData[tile.get()] = val;
      });

      // Compute convolution
      parallel_for_work_item(group, [&](item<1> tile)
      {
        float sum = 0.f;
        for (unsigned offset = 0; offset < radius; ++offset) {
          sum += filterLocal[offset] * localData[tile + id<1>{offset + radius}];
        }
        float result = sum / (2 * radius + 1);
        convolutionResult[tile.get()] = result;
      });

      // Store result data
      parallel_for_work_item(group, [&](item<1> tile)
      {
        // From the input data offset by the beginning of this group's block
        // and add the iteration
        out_access[group.get() * id<1>{blocksize} + id<1>{tile}] = convolutionResult[tile.get()];
      });
    });
  });

  auto hostData = outputB.get_access<access::read>();
  
  std::cout << "Input:\n";
  for (int i = 0; i < size; ++i) {
    if (i > 0) std::cout << ", ";
    std::cout << data[i];
  }
  std::cout << "\n";

  std::cout << "Filter:\n";
  for (int i = 0; i < (2*radius+1); ++i) {
    if (i > 0) std::cout << ", ";
    std::cout << filter[i];
  }
  std::cout << "\n";

  std::cout << "Output:\n";
  for (int i = 0; i < size; ++i) {
    if (i > 0) std::cout << ", ";
    std::cout << hostData[i];
  }
  std::cout << "\n";
  
  return 0;
}
