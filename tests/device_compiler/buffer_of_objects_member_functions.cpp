#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <vector>

#include <boost/test/minimal.hpp>

/*
  The aim of this test is to check that C Struct objects can be used with
  buffers (e.g. buffers of objects) and that function members can be accessed
  and used to modify variables.

  Currently does not compile with XOCL when targeting hw_emu, address space
  related bug.
*/

using namespace cl::sycl;

constexpr size_t N = 32;

struct Index_Member_Data {
private:
  std::size_t g;
  std::size_t r;

public:
  Index_Member_Data() : g(0), r(0) {};

  auto get_g() { return g; }
  auto get_r() { return r; }
  void set_g(std::size_t g_) { g = g_; }
  void set_r(std::size_t r_) { r = r_; }
};

// check if standard layout and trivially copyable before execution, i.e. it's
// POD (the type_trait is_pod is going to be deprecated in C++20)
static_assert(std::is_standard_layout_v<Index_Member_Data> &&
              std::is_trivially_copyable_v<Index_Member_Data>,
              "Index_Member_Data is not POD");

int test_main(int argc, char *argv[]) {
  queue my_queue{default_selector{}};

  buffer<Index_Member_Data, 1> a{N};

  auto acc_w = a.get_access<access::mode::write>();
  for (unsigned int i = 0; i < N; ++i) {
    acc_w[i] = Index_Member_Data{};
  }

  my_queue.submit([&](handler &cgh) {
    auto acc = a.get_access<access::mode::write>(cgh);

    cgh.parallel_for<class buffer_of_objects_member_func>(
        range<1>{N}, [d_a = drt::accessor<decltype(acc)>{acc}](item<1> index) {
           d_a[index.get_id(0)].set_g(index.get_id(0));
           d_a[index.get_id(0)].set_r(index.get_range()[0]);
        });
  });

  my_queue.wait();

  auto acc_r = a.get_access<access::mode::read>();

  for (unsigned int i = 0; i < N; ++i) {
    BOOST_CHECK(acc_r[i].get_r() == 32);
    BOOST_CHECK(acc_r[i].get_g() == i);
  }

  return 0;
}
