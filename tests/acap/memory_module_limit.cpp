/* Testing the AI Engine Memory Module boundary geometry

   RUN: %{execute}%s
*/

#include <sycl/sycl.hpp>

#include <iostream>
#include <type_traits>

#include <boost/test/minimal.hpp>

using namespace sycl::vendor::xilinx;

template <typename Layout>
struct test_memory {

  // Model the existence of a memory bank
  struct status {
    bool west;
    bool east;
    bool south;
    bool north;
  };

  static auto constexpr x_size = Layout::x_max + 1;
  static auto constexpr y_size = Layout::y_max + 1;

  static inline status mma[y_size][x_size];

  template <typename AIE, int X, int Y>
  struct tile : acap::aie::tile<AIE, X, Y> {
    using t = acap::aie::tile<AIE, X, Y>;

    void run() {
      auto &s = mma[Y][X];
      s.west = t::is_memory_module_west();
      s.east = t::is_memory_module_east();
      s.south = t::is_memory_module_south();
      s.north = t::is_memory_module_north();
    }
  };

  test_memory() {
    acap::aie::device<Layout> {}.template run<tile, acap::aie::memory>();
    /* Check that the existence of the memory module fits the bondary
       geometry */
    for (int y = 0; y < y_size; ++y)
      for (int x = 0; x < x_size; ++x) {
        auto &s = mma[y][x];
        if (y == 0)
          BOOST_CHECK(!s.south);
        else
          BOOST_CHECK(s.south);
        if (y == Layout::y_max)
          BOOST_CHECK(!s.north);
        else
          BOOST_CHECK(s.north);
        if (x == 0 && !(y & 1))
          BOOST_CHECK(!s.west);
        else
          BOOST_CHECK(s.west);
        if (x == Layout::x_max && (y & 1))
          BOOST_CHECK(!s.east);
        else
          BOOST_CHECK(s.east);
      }
  }
};

int test_main(int argc, char *argv[]) {
  // Test for various machine sizes
  test_memory<acap::aie::layout::one_pe> tm_1;
  test_memory<acap::aie::layout::small> tm_small;
  test_memory<acap::aie::layout::full> tm_full;

  return 0;
}
