/* Testing the MathEngine Memory Module boundary geometry

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
   Based on Math Engine (ME) Architecture Specification, Revision v1.4
   March 2018

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>
#include <type_traits>

#include <boost/test/minimal.hpp>

using namespace cl::sycl::vendor::xilinx;

template <typename Layout>
struct test_memory {

  // Model the existence of a memory bank
  struct status {
    bool left;
    bool right;
    bool down;
    bool up;
  };

  static auto constexpr x_size = Layout::x_max + 1;
  static auto constexpr y_size = Layout::y_max + 1;

  static inline status mma[y_size][x_size];

  template <typename ME_Array,
            int X,
            int Y>
  struct tile : acap::me::tile<ME_Array,
                               X,
                               Y> {
    using t = acap::me::tile<ME_Array, X, Y>;

    void run() {
      auto &s = mma[Y][X];
      s.left = t::is_memory_module_left();
      s.right = t::is_memory_module_right();
      s.down = t::is_memory_module_down();
      s.up = t::is_memory_module_up();
    }
  };

  test_memory() {
    acap::me::array<Layout, tile, acap::me::memory> me;
    me.run();
    /* Check that the existence of the memory module fits the bondary
       geometry */
    for (int y = 0; y < y_size; ++y)
      for (int x = 0; x < x_size; ++x) {
        auto &s = mma[y][x];
        if (y == 0)
          BOOST_CHECK(!s.down);
        else
          BOOST_CHECK(s.down);
        if (y == Layout::y_max)
          BOOST_CHECK(!s.up);
        else
          BOOST_CHECK(s.up);
        if (x == 0 && !(y & 1))
          BOOST_CHECK(!s.left);
        else
          BOOST_CHECK(s.left);
        if (x == Layout::x_max && (y & 1))
          BOOST_CHECK(!s.right);
        else
          BOOST_CHECK(s.right);
      }
  }
};

int test_main(int argc, char *argv[]) {
  // Test for various machine sizes
  test_memory<acap::me::layout::one_pe> tm_1;
  test_memory<acap::me::layout::small> tm_small;
  test_memory<acap::me::layout::full> tm_full;

  return 0;
}
