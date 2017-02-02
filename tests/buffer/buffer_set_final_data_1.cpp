/* RUN: %{execute}%s

   Exercise buffer::set_final_data()
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  constexpr int N = 16;
  {
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);
    std::vector<int> w(N);

    {
      buffer<int> mybuffer { v.begin(), v.end() };
      mybuffer.set_final_data(w.begin());
    }

    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == 0);
    }
  }
  {
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);
    std::vector<int> w(N);

    {
      buffer<int> mybuffer { v.begin(), v.end() };
      mybuffer.mark_as_written();
      mybuffer.set_final_data(w.begin());
    }

    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == i);
    }
  }
  {
    std::vector<int> v(N);
    std::vector<int> w(N);

    {
      buffer<int> mybuffer { v.begin(), v.end() };
      queue {}.submit([&](handler &cgh) {
          auto write = mybuffer.get_access<access::mode::write>(cgh);

          cgh.parallel_for<class generate>(range<1> { N },
                                           [=] (id<1> index) {
                                             write[index] = index;
                                           });
      });
      mybuffer.set_final_data(w.begin());
    }

    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == i);
    }
  }
  {
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);
    std::vector<int> w(N);

    {
      buffer<int> mybuffer { v.begin(), v.end() };
      auto read = mybuffer.get_access<access::mode::read, access::target::host_buffer>();
      for (int i = 0; i < N; ++i)
        std::cout << read[i] << ", ";
      std::cout << std::endl;
      mybuffer.set_final_data(w.begin());
    }

    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == 0);
    }
  }

  return 0;
}
