/* Testing the MathEngine Memory Module with locking mechanism

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
   Based on Math Engine (ME) Architecture Specification, Revision v1.4
   March 2018

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>
#include <type_traits>

using namespace cl::sycl::vendor::xilinx;

// All the memory modules are the same
template <typename ME_Array, int X, int Y>
struct memory : acap::me::memory<ME_Array, X, Y> {
  int v;
};

// By default all the tiles have an empty program
template <typename ME_Array, int X, int Y>
struct tile : acap::me::tile<ME_Array, X, Y> {};

// The tile(0,0) write in memory module on the right
template <typename ME_Array>
struct tile<ME_Array, 0, 0> : acap::me::tile<ME_Array, 0, 0> {
  using t = acap::me::tile<ME_Array, 0, 0>;

  void run() {
    auto &m = t::mem_right();
    m.v = 42;
    for (int i = 0; i < 100; ++i) {
      m.lu.locks[0].wait_value(false);
      ++m.v;
      std::cout << std::endl << "Tile (0,0) sends to right neighbour: "
                << m.v << std::endl;
      m.lu.locks[0].release_value(true);
    }
  }
};

// The tile(1,0) read from memory module on the leftt
template <typename ME_Array>
struct tile<ME_Array, 1, 0> : acap::me::tile<ME_Array, 1, 0> {
  using t = acap::me::tile<ME_Array, 1, 0>;

  void run() {
    auto &m = t::mem_left();
    for (int i = 0; i < 100; ++i) {
      m.lu.locks[0].wait_value(true);
      std::cout << "Tile (1,0) receives from left neighbour: "
                << m.v << std::endl;
      m.lu.locks[0].release_value(false);
    }
  }
};

int main(int argc, char *argv[]) {
  std::cout << std::endl << "Instantiate small MathEngine:"
            << std::endl << std::endl;
  acap::me::array<acap::me::layout::small,
                  tile,
                  memory> me;

  graphics::app a { argc, argv, decltype(me)::geo::x_size,
      decltype(me)::geo::y_size, 1, 1, 100 };

  // Launch the MathEngin program
  me.run();

  std::uint32_t pixel = 0x778899aa;
  a.update_tile_data_image(1, 2, (unsigned char *)&pixel);
  pixel = ~0x778899aa;
  a.update_tile_data_image(0, 1, (unsigned char *)&pixel);
  // Wait for the graphics to stop
  a.join();
}
