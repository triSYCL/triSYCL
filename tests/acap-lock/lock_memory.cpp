/* Testing the AI Engine Memory Module with locking mechanism

   RUN: %{execute}%s
*/

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>

#include <sycl/sycl.hpp>
#include "triSYCL/vendor/Xilinx/graphics.hpp"

using namespace std::chrono_literals;
using namespace sycl::vendor::xilinx;

graphics::application a;

// All the memory modules are the same
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  int v;
};

// By default all the tiles have an empty program
template <typename AIE, int X, int Y>
struct tile : acap::aie::tile<AIE, X, Y> {};

// The tile(0,0) write in memory module to the East
template <typename AIE>
struct tile<AIE, 0, 0> : acap::aie::tile<AIE, 0, 0> {
  using t = acap::aie::tile<AIE, 0, 0>;

  void run() {
    auto &m = t::mem_east();
    m.v = 42;
    for (int i = 0; i < 100; ++i) {
      m.lock(0).acquire_with_value(false);
      ++m.v;
      a.update_tile_data_image(t::x, t::y, &m.v, 42, 143);
      std::cout << std::endl << "Tile (0,0) sends to East neighbour: "
                << m.v << std::endl;
      m.lock(0).release_with_value(true);
      std::this_thread::sleep_for(20ms);
    }
  }
};

// The tile(1,0) read from memory module to the West
template <typename AIE>
struct tile<AIE, 1, 0> : acap::aie::tile<AIE, 1, 0> {
  using t = acap::aie::tile<AIE, 1, 0>;

  void run() {
    auto &m = t::mem_west();
    for (int i = 0; i < 100; ++i) {
      m.lock(0).acquire_with_value(true);
      a.update_tile_data_image(t::x, t::y, &m.v, 42, 143);
      std::cout << "Tile (1,0) receives from West neighbour: "
                << m.v << std::endl;
      m.lock(0).release_with_value(false);
      std::this_thread::sleep_for(20ms);
    }
  }
};

int main(int argc, char *argv[]) {
  std::cout << std::endl << "Instantiate small AI Engine:"
            << std::endl << std::endl;
  acap::aie::device<acap::aie::layout::small> aie;

  a.start(argc, argv, decltype(aie)::geo::x_size,
          decltype(aie)::geo::y_size, 1, 1, 100);

  // Launch the AI Engine program
  aie.run<tile, memory>();
}
