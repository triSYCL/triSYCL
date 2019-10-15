/* Simple hello world program for ACAP/AI Engine hardware

   NOTE: This requires tweaking to work with the current compiler, for example
   the tile functions relating to reset/etc are pushed down into the API and the
   kernel doesn't need to be commented out. But also some more lower level
   components like the linker script have changed and the main file (most of the
   earlier iterations were a hybrid of Cardano Tool Flow and SYCL Frontend flow)

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl.hpp>

using namespace sycl::vendor::xilinx;

/** A small AI Engine program

    The definition of a tile program has to start this way

    \param AIE is an implementation-defined type

    \param X is the horizontal coordinate

    \param Y is the vertical coordinate
*/
template <typename AIE, int X, int Y>
struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  /// The run member function is defined as the tile program
  void run() {
    std::cout << "Hello, I am the AIE tile (" << X << ',' << Y  << ")"
              << std::endl;
    // Write some data to memory
    for (unsigned int i = 0; i < 16; i++) {
      t::mem_write(i * 0x4, i * 2);
      std::cout << t::mem_read(i * 0x4) << std::endl;
    }
    // overwrite with identifiers
    t::mem_write(0, X);
    t::mem_write(0x4, Y);
  }
};

int main() {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> aie;
  // Run up to completion of all the tile programs
  aie.run<prog>();
}
