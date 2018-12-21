#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/test/minimal.hpp>

/*
  The aim of this test is to check that multidimensional kernels are executing
  appropriately using OpenCL ND range kernels and outputting correct
  information.
*/
using namespace cl::sycl;

template <int Dimensions, class kernel_name>
void gen_nd_range(range<Dimensions> k_range) {
  queue my_queue{default_selector{}};

  buffer<unsigned int> a(k_range.size());

  my_queue.submit([&](handler &cgh) {
    auto acc = a.get_access<access::mode::discard_write>(cgh);

    cgh.parallel_for<kernel_name>(k_range,
        [d_a = drt::accessor<decltype(acc)>{acc}](item<Dimensions> index) {
            unsigned int range = index.get_range()[0];
            for (size_t i = 1; i < Dimensions; ++i)
              range *= index.get_range()[i];

            d_a[index.get_linear_id()] = index.get_linear_id() + range;
        });
  });

  auto acc_r = a.get_access<access::mode::read>();

  for (unsigned int i = 0; i < k_range.size(); ++i) {
    BOOST_CHECK(acc_r[i] == k_range.size() + i);
  }

  my_queue.wait();
}

int test_main(int argc, char *argv[]) {
  gen_nd_range<1, class par_1d>({10});
  gen_nd_range<2, class par_2d_square>({10, 10});
  gen_nd_range<2, class par_2d_square>({12, 12});
  gen_nd_range<2, class par_2d_rect>({12, 6});
  gen_nd_range<3, class par_3d_square>({10, 10, 10});
  gen_nd_range<3, class par_3d_rect>({12, 8, 16});
  gen_nd_range<1, class par_1d>({1000});
  gen_nd_range<2, class par_2d_square>({100, 100});
  gen_nd_range<2, class par_2d_rect>({120, 120});
  gen_nd_range<2, class par_2d_rect>({120, 60});
  gen_nd_range<3, class par_3d_square>({100, 100, 100});
  gen_nd_range<3, class par_3d_rect>({150, 200, 150});
  gen_nd_range<3, class par_3d_rect>({150, 200, 100});

  return 0;
}
