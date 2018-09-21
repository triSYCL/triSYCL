/* Testing the MathEngine Memory Module with locking mechanism

   Based on Math Engine (ME) Architecture Specification, Revision v1.4
   March 2018

   RUN: %{execute}%s
*/

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>

#include <CL/sycl.hpp>

using namespace std::chrono_literals;
using namespace cl::sycl::vendor::xilinx;

std::unique_ptr<graphics::app> a;

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
      m.lu.locks[0].acquire_with_value(false);
      ++m.v;
      a->update_tile_data_image(t::x, t::y, &m.v, 42, 143);
      std::cout << std::endl << "Tile (0,0) sends to right neighbour: "
                << m.v << std::endl;
      m.lu.locks[0].release_with_value(true);
      std::this_thread::sleep_for(20ms);
    }
  }
};

// The tile(1,0) read from memory module on the left
template <typename ME_Array>
struct tile<ME_Array, 1, 0> : acap::me::tile<ME_Array, 1, 0> {
  using t = acap::me::tile<ME_Array, 1, 0>;

  void run() {
    auto &m = t::mem_left();
    for (int i = 0; i < 100; ++i) {
      m.lu.locks[0].acquire_with_value(true);
      a->update_tile_data_image(t::x, t::y, &m.v, 42, 142);
      std::cout << "Tile (1,0) receives from left neighbour: "
                << m.v << std::endl;
      m.lu.locks[0].release_with_value(false);
      std::this_thread::sleep_for(20ms);
    }
  }
};

int main(int argc, char *argv[]) {
  std::cout << std::endl << "Instantiate small MathEngine:"
            << std::endl << std::endl;
  acap::me::array<acap::me::layout::small,
                  tile,
                  memory> me;

  a.reset(new graphics::app { argc, argv, decltype(me)::geo::x_size,
                              decltype(me)::geo::y_size, 1, 1, 100 });

  // Launch the MathEngine program
  me.run();
/*
  std::int32_t pixel = 2;
  a.update_tile_data_image(1, 2, &pixel, 0, 8);
  pixel = 7;
  a.update_tile_data_image(1, 2, &pixel, 0, 8);
*/
 // Wait for the graphics to stop
  a->wait();
}
