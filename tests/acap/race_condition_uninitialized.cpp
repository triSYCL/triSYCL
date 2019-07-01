/* Testing the tools to detect race condition and uninitialized memory access

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl.hpp>

using namespace sycl::vendor::xilinx;

// All the memory modules are the same
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  float uninitialized;
  int v[2];
  float overflow;
};

/** Default program. This will be tile(0,0) only.

    The tile(0,0) uses memory module on the right */
template <typename AIE, int X, int Y>
struct program : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  void run() {
    auto &m = t::mem_right();
    // Unprotected access in respect to tile(1,0)
    m.v[0] = 42;

    // Protected data transfer
    m.lu.locks[0].acquire_with_value(false);
    m.v[1] = 1;
    m.lu.locks[0].release_with_value(true);
  }
};

/** Specialize for the tile(1,0).

    The tile(1,0) uses memory module on the left */
template <typename AIE>
struct program<AIE, 1, 0> : acap::aie::tile<AIE, 1, 0> {
  using t = acap::aie::tile<AIE, 1, 0>;

  void run() {
    auto &m = t::mem_left();
    // Unprotected access in respect to tile(0,0)
    m.v[0] = 314;

    // Protected data transfer
    m.lu.locks[0].acquire_with_value(true);
    std::cout << "Tile (1,0) receives from left neighbour: "
              << m.v[1] << std::endl;
    m.lu.locks[0].release_with_value(false);

    // Oops! Array overrun: writing into m.v[2]

    m.v[m.v[1]+1] = 0xDeadBeef;

    std::cout << "Tile (1,0) read overflow: "
                << m.overflow << std::endl;
    std::cout << "Tile (1,0) read uninitialized: "
                << m.uninitialized << std::endl;
  }
};

int main(int argc, char *argv[]) {
  acap::aie::array<acap::aie::layout::size<2,1>, program, memory> {}.run();
}
