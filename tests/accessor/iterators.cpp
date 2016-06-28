/* RUN: %{execute}%s

   Experiment with iterators on buffer accessors
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <iterator>
#include <numeric>

#include <boost/iterator/zip_iterator.hpp>
#include <boost/test/minimal.hpp>
// Actually zip_iterator.hpp requires this implementation of tuples...
#include <boost/tuple/tuple.hpp>

constexpr size_t N = 300;
using Type = int;

int test_main(int argc, char *argv[]) {
  // Initialize the input buffers to some easy-to-compute values
  cl::sycl::buffer<Type> a { N };
  {
    auto aa = a.get_access<cl::sycl::access::mode::write>();
    // Initialize buffer a with increasing integer starting at 0
    std::iota(aa.begin(), aa.end(), 0);
  }
  cl::sycl::buffer<Type> b { N };
  {
    auto ab = b.get_access<cl::sycl::access::mode::write>();
    // Initialize buffer b starting from the end with increasing
    // integer starting at 42
    std::iota(ab.rbegin(), ab.rend(), 42);
  }

  // A buffer of N Type to get the result
  cl::sycl::buffer<Type> c { N };

  // Create a queue to launch the kernel
  cl::sycl::queue q;

  // Launch a kernel to do the summation
  q.submit([&] (cl::sycl::handler &cgh) {
      // Get access to the data
      auto aa = a.get_access<cl::sycl::access::mode::read>(cgh);
      auto ab = b.get_access<cl::sycl::access::mode::read>(cgh);
      auto ac = c.get_access<cl::sycl::access::mode::write>(cgh);

      cgh.single_task<class sum>([=] {
          // For fun, use aggregate iterators
          auto b = boost::make_zip_iterator(boost::make_tuple(aa.cbegin(),
                                                              ab.cbegin(),
                                                              ac.begin()));
          auto e = boost::make_zip_iterator(boost::make_tuple(aa.cend(),
                                                              ab.cend(),
                                                              ac.end()));

          for (auto i = b; i != e; ++i)
            i->get<2>() = i->get<0>() + i->get<1>();
        });
    });

  //std::cout << std::endl << "Result:" << std::endl;
  for (auto e : c.get_access<cl::sycl::access::mode::read>())
    BOOST_CHECK(e == N + 42 - 1);
    // std::cout << e << " ";
  //std::cout << std::endl;

  return 0;
}
