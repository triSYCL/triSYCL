/* Testing the locks in an AI Engine Memory Module neigborhood

   This also describes the AIE tile memory layout, according to row parity.

   RUN: %{execute}%s
*/

#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

/* Programs which propagate an acquire/release pattern in a square
   neighborhood */

// The template define the core tile program by default, so tile(0,0) here
template <typename AIE, int X, int Y> struct tile : acap::aie::tile<AIE, X, Y> {
  void operator()() {
    // Wait for notification from the host
    this->mem().lock(0).acquire_with_value(true);
    // Send notification to tile (1,0)
    this->mem_east().lock(1).release_with_value(true);
  }
};

template <typename AIE> struct tile<AIE, 1, 0> : acap::aie::tile<AIE, 1, 0> {
  void operator()() {
    // Wait for notification from tile (0,0)
    this->mem_west().lock(1).acquire_with_value(true);
    // Send notification to tile (1,1)
    this->mem_north().lock(2).release_with_value(true);
  }
};

template <typename AIE> struct tile<AIE, 0, 1> : acap::aie::tile<AIE, 0, 1> {
  void operator()() {
    // Wait for notification from tile (1,1)
    this->mem_east().lock(3).acquire_with_value(true);
    // Send notification to the host
    this->mem().lock(4).release_with_value(true);
  }
};

template <typename AIE> struct tile<AIE, 1, 1> : acap::aie::tile<AIE, 1, 1> {
  void operator()() {
    // Wait for notification from tile (1,0)
    this->mem().lock(2).acquire_with_value(true);
    // Send notification to tile (0,1)
    this->mem_west().lock(3).release_with_value(true);
  }
};

int main(int argc, char* argv[]) {
  acap::aie::device<acap::aie::layout::size<2, 2>> d;
  // launch the program made of tile
  auto aie_future = d.queue().submit<tile>();
  // Notify the memory module of tile(0,0)
  d.mem(0,0).lock(0).release_with_value(true);
  // Wait for the tile(0,1)
  // Short-cut API
  // d.mem(0,1).lock(4).acquire_with_value(true);
  // Alternative API
  d.tile(0,1).mem().lock(4).acquire_with_value(true);
  // Wait for all the tiles to complete
  aie_future.get();
}
