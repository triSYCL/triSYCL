#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include <boost/test/minimal.hpp>

/*
  A contrived test with the aim of testing that C++ objects can be created and
  destroyed inside a kernel implicitly and to detect any problems with address
  spaces that may occur in the process.
*/

using namespace cl::sycl;

constexpr size_t N = 32;

class Index_Data {
public:
  Index_Data() : g(0) {}
  ~Index_Data() {}

  std::size_t g;
};

class Index_Member_Data {
  std::size_t g;

public:
  Index_Member_Data() : g(0) {}
  ~Index_Member_Data() {}

  auto get_g() const { return g; }
  void set_g(std::size_t g_) { g = g_; }
};

int test_main(int argc, char *argv[]) {

    queue my_queue{default_selector{}};

    buffer<unsigned int, 1> a{N};

    auto acc_w = a.get_access<access::mode::write>();
    for (unsigned int i = 0; i < N; ++i) {
      acc_w[i] = 0;
    }

    Index_Member_Data idm_outer;
    idm_outer.set_g(20);

    my_queue.submit([&](handler &cgh) {
      auto acc = a.get_access<access::mode::write>(cgh);

      Index_Member_Data idm_inner;
      idm_inner.set_g(10);

      cgh.single_task<class ctor_dtor>(
        [=, d_a = drt::accessor<decltype(acc)>{acc}] {
            for (unsigned int i = 0 ; i < N; ++i) {
              Index_Data id;
              id.g = i + idm_inner.get_g() + idm_outer.get_g();
              d_a[i] = id.g;
            }
        });
    });

    my_queue.wait();

    auto acc_r = a.get_access<access::mode::read>();

    for (unsigned int i = 0; i < N; ++i) {
      BOOST_CHECK(acc_r[i] == i + 30);
    }

    return 0;
}
