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
    // v = { 0, 1, 2, ..., N-1 };
    std::iota(v.begin(), v.end(), 0);
    // w = { 0, ..., 0 };
    std::vector<int> w(N);

    {
      buffer<int> buff { v.begin(), v.end() };
      buff.set_final_data(w.begin());
    }

    // Tthe buffer is not modified, thus w = { 0, ..., 0 };
    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(v[i] == i);
      BOOST_CHECK(w[i] == 0);
    }
  }
  {
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);
    std::vector<int> w(N);

    {
      buffer<int> buff { v.begin(), v.end() };
      buff.mark_as_written();
      buff.set_final_data(w.begin());
    }

    // The buffer is 'modified', thus w = v;
    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(v[i] == i);
      BOOST_CHECK(w[i] == i);
    }
  }
  {
    std::vector<int> v(N);
    std::vector<int> w(N);

    {
      buffer<int> buff { v.begin(), v.end() };
      queue {}.submit([&](handler &cgh) {
          auto write = buff.get_access<access::mode::write>(cgh);

          cgh.parallel_for<class generate>(range<1> { size_t(N) }, // MSVC: error C2398: Element '1': conversion from 'const int' to 'size_t' requires a narrowing conversion
                                           [=] (id<1> index) {
                                             write[index] = index[0];
                                           });
      });
      buff.set_final_data(w.begin());
    }

    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(v[i] == 0);
      BOOST_CHECK(w[i] == i);
    }
  }
  {
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);
    std::vector<int> w(N);

    {
      buffer<int> buff { v.begin(), v.end() };
      auto read = buff.get_access<access::mode::read>();
      buff.set_final_data(w.begin());
    }

    for (int i = 0; i < N; ++i) {
      // std::cerr << w[i] << ':' << i << std::endl;
      BOOST_CHECK(v[i] == i);
      BOOST_CHECK(w[i] == 0);
    }
  }
  {
    std::vector<int> v(N);

    {
      buffer<int> buff { v.begin(), v.end() };
      buff.set_final_data(v.begin());
      auto write = buff.get_access<access::mode::write>();
      for (int i = 0; i < N; ++i)
        write[i] = i;
    }

    //A write accessor is created thus a write-back is triggered on v.
    for (int i = 0; i < N; ++i) {
      // std::cout << i << " -> " << v[i] << std::endl;
      BOOST_CHECK(v[i] == i);
    }
  }
  {
    std::vector<int> v(N);

    {
      buffer<int> buff { v.cbegin(), v.cend() };
      auto write = buff.get_access<access::mode::write>();
      for (int i = 0; i < N; ++i)
        write[i] = i;
    }

    /* A write accessor is created but no write_back is specified thus no
       write-back is triggered.
     */
    for (int i = 0; i < N; ++i) {
      // std::cout << i << " -> " << v[i] << std::endl;
      BOOST_CHECK(v[i] == 0);
    }
  }

  return 0;
}
