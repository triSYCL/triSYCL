/* Simple hello world program for ACAP/AI Engine

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
  /// The run member function is defined as the tile program
  void run() {
    std::cout << "Hello, I am the AIE tile (" << X << ',' << Y  << ")"
              << std::endl;
  }
};

int main() {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  acap::aie::array<acap::aie::layout::vc1902, prog> aie;
  // Run up to completion of all the tile programs
  aie.run();
}
