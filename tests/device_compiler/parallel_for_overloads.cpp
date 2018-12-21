#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/test/minimal.hpp>

/*
  The aim of this test is to check the parallel_for overloads translate down to
  an ND_RANGE_KERNEL invocation using schedule_parallel_for without problem.
  Passing the correctly constructed item classes.

  It currently doesn't fully detect the accuracy of the information stored in
  these classes or Dimensions > 1.
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

    cgh.parallel_for<class parallel_overload_0>(
        range<1>{N}, [d_a = drt::accessor<decltype(acc)>{acc}](id<1> index) {
          d_a[index.get(0)] += index.get(0);
        });
  });

  my_queue.wait();

  my_queue.submit([&](handler &cgh) {
    auto acc = a.get_access<access::mode::write>(cgh);

    cgh.parallel_for<class parallel_overload_1>(
        range<1>{N}, [d_a = drt::accessor<decltype(acc)>{acc}](item<1> index) {
          d_a[index.get_id(0)] += index[0];
        });
  });

  my_queue.wait();

  my_queue.submit([&](handler &cgh) {
    auto acc = a.get_access<access::mode::write>(cgh);

    cgh.parallel_for<class parallel_overload_2>(
        N, [d_a = drt::accessor<decltype(acc)>{acc}](item<1> index) {
          d_a[index.get_id(0)] += index.get_id(0);
        });
  });

  my_queue.wait();

  // Unimplemented at the moment I believe, useful future test though
  // my_queue.submit([&](handler &cgh) {
  // auto acc = a.get_access<access::mode::write>(cgh);
  // cgh.parallel_for<class parralel_overload_3>(nd_range<1>{range<1>{N}},
  //     [d_a = drt::accessor<decltype(acc)>{acc}](nd_item<1> index) {
  //      d_a[index.get_id(0)] = index.get_id(0);
  // });
  // });

  // my_queue.wait();

  auto acc_r = a.get_access<access::mode::read>();

  for (unsigned int i = 0; i < N; ++i) {
    BOOST_CHECK(acc_r[i] == i*3);
  }

  return 0;
}
