/* RUN: %{execute}%s

   Experiment with iterators on buffer accessors
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>
#include <numeric>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

constexpr size_t N = 300;
using Type = int;


int test_main(int argc, char *argv[]) {
  // The convolution kernel
  const std::vector<Type> conv_kernel_init = { 1, 2, 4 };
  buffer<Type> conv_kernel { conv_kernel_init.begin(),
                             conv_kernel_init.end() };

  // Use the fact it is initialized to 0 and just allocate what is needed
  std::vector<Type> gold_result(N - (conv_kernel_init.size() - 1));

  // The input values to be convolved
  buffer<Type> a { N };
  {
    // Initialize the input buffers to some easy-to-compute values
    auto a_a = a.get_access<access::mode::read_write>();
    // Initialize buffer a with increasing integer starting at 0
    std::iota(a_a.begin(), a_a.end(), 0);

    // Compute the reference convolution
    for (unsigned int i = 0 ; i < gold_result.size(); ++i)
      for (unsigned int j = 0; j < conv_kernel_init.size(); ++j)
        gold_result[i] += a_a[i + j]*conv_kernel_init[j];
  }

  buffer<Type> result { gold_result.size() };

  // Create a queue to launch the kernel
  queue q;

  // Launch a kernel to do the summation
  q.submit([&] (handler &cgh) {
      // Get access to the data
      auto a_a = a.get_access<access::mode::read>(cgh);
      auto a_conv_kernel = conv_kernel.get_access<access::mode::read>(cgh);
      // A cache conv_kernel in local memory
      accessor<decltype(a_conv_kernel)::value_type,
               1,
               access::mode::read_write,
               access::target::local> a_conv_cache { conv_kernel.get_count(),
                                                     cgh };
      auto a_result = result.get_access<access::mode::write>(cgh);

          std::cerr << N - (conv_kernel.get_count() - 1) << std::endl;
      // Launch 1 work-group with N - (conv_kernel.get_count() - 1 work-items
      cgh.parallel_for_work_group<class convol>(
        nd_range<1> {
          N - (conv_kernel.get_count() - 1),
          N - (conv_kernel.get_count() - 1) },
        [=](group<1> g) {
          g.parallel_for_work_item([&](h_item<1> i) {
              // Cache the convolution kernel in local memory
              if (i.get_local_id(0) < a_conv_kernel.get_count())
                a_conv_cache[i.get_local_id(0)] =
                  a_conv_kernel[i.get_global_id()];
            });
          // An implicit barrier happens here
          g.parallel_for_work_item([&](h_item<1> i) {
              a_result[i.get_global_id()] = 0;
              // The convolution
              for (unsigned int j = 0; j < conv_kernel.get_count(); ++j)
                a_result[i.get_global_id()] +=
                  a_a[i.get_local_id(0) + j]*a_conv_cache[j];
            });
        });
    });

  auto a_result = result.get_access<access::mode::read>();
  // Verify the result
  for (unsigned int i = 0 ; i < gold_result.size(); ++i)
    BOOST_CHECK(gold_result[i] == a_result[i]);

  return 0;
}
