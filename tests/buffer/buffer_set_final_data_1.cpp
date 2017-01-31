/* RUN: %{execute}%s

   Exercise buffer::set_final_data()
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

int test_main(int argc, char *argv[]) {
  constexpr size_t N = 16;
  {
    std::vector<int> v (N);
    for(size_t i = 0; i < N; i++)
      v[i] = i;
    std::vector<int> w (N);

    {
      buffer<int> result {v.begin(), v.end()};
      result.set_final_data<typename std::vector<int>::iterator >(w.begin());
    }

    for (int i = 0; i != N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == 0);
    }
  }
  {
    std::vector<int> v (N);
    for(size_t i = 0; i < N; i++)
      v[i] = i;
    std::vector<int> w (N);

    {
      buffer<int> result {v.begin(), v.end()};
      result.mark_as_written();
      result.set_final_data(w.begin());
    }

    for (int i = 0; i != N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == i);
    }
  }
  {
    std::vector<int> v (N);
    std::vector<int> w (N);

    {
      buffer<int> result {v.begin(), v.end()};
      queue {}.submit([&](handler &cgh) {
          auto write = result.get_access<access::mode::write>(cgh);

          cgh.parallel_for<class generate>(range<1> { N },
                                           [=] (id<1> index) {
                                             write[index] = index;
                                           });
      });
      result.set_final_data(w.begin());
    }

    for (int i = 0; i != N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == i);
    }
  }
  {
    std::vector<int> v (N);
    for(size_t i = 0; i < N; i++)
      v[i] = i;
    std::vector<int> w (N);

    {
      buffer<int> result {v.begin(), v.end()};
      auto read = result.get_access<access::mode::read, access::target::host_buffer>();
      for(size_t i = 0; i < N; i++)
        std::cout << read[i] << ", ";
      std::cout << std::endl;
      result.set_final_data(w.begin());
    }

    for (int i = 0; i != N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(w[i] == 0);
    }
  }

  return 0;
}
