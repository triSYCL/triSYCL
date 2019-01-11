#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/test/minimal.hpp>

/*
  The aim of this test is to check that C Struct objects can be used with
  buffers (e.g. buffers of objects) and that the data members can be accessed
  and modified trivially.
*/

using namespace cl::sycl;

constexpr size_t N = 32;

struct Index_Data {
  Index_Data() : g(0), r(0) {};

  std::size_t g;
  std::size_t r;
};

// check if standard layout and trivially copyable before execution, i.e. it's
// POD (the type_trait is_pod is going to be deprecated in C++20)
static_assert(std::is_standard_layout_v<Index_Data> &&
              std::is_trivially_copyable_v<Index_Data>,
              "Index_Data is not POD");

int test_main(int argc, char *argv[]) {
  queue my_queue{default_selector{}};

  buffer<Index_Data, 1> a{N};

  auto acc_w = a.get_access<access::mode::write>();
  for (unsigned int i = 0; i < N; ++i) {
    acc_w[i] = Index_Data{};
  }

  my_queue.submit([&](handler &cgh) {
    auto acc = a.get_access<access::mode::write>(cgh);

    cgh.parallel_for<class buffer_of_objects_member_data>(
        range<1>{N}, [d_a = drt::accessor<decltype(acc)>{acc}](item<1> index) {
           d_a[index.get_id(0)].g = index.get_id(0);
           d_a[index.get_id(0)].r = index.get_range()[0];
        });
  });

  my_queue.wait();

  auto acc_r = a.get_access<access::mode::read>();

  for (unsigned int i = 0; i < N; ++i) {
    BOOST_CHECK(acc_r[i].r == 32);
    BOOST_CHECK(acc_r[i].g == i);
  }

  return 0;
}
