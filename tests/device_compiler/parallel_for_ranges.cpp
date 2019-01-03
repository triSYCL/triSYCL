#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/test/minimal.hpp>

/*
  The aim of this test is to check that the range information is correct at a
  basic level. The parallel_for_ND_range.cpp test does it a little more
  comprehensively perhaps.
*/
using namespace cl::sycl;

constexpr size_t N = 32;

int test_main(int argc, char *argv[]) {
  queue my_queue{default_selector{}};

  buffer<unsigned int, 1> a{N};

  auto acc_w = a.get_access<access::mode::write>();
  for (unsigned int i = 0; i < N; ++i) {
    acc_w[i] = 0;
  }

  my_queue.submit([&](handler &cgh) {
    auto acc = a.get_access<access::mode::write>(cgh);

    cgh.parallel_for<class parallel_overload_1>(
        range<1>{N}, [d_a = drt::accessor<decltype(acc)>{acc}](item<1> index) {
          d_a[index.get_id(0)] += index.get_range()[0];
        });
  });

  my_queue.wait();

  auto acc_r = a.get_access<access::mode::read>();

  for (unsigned int i = 0; i < N; ++i) {
    BOOST_CHECK(acc_r[i] == 32);
  }

  return 0;
}
