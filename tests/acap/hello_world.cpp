/* Simple hello world program for ACAP/AI Engine

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl/sycl.hpp>

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
  // Define AIE CGRA with all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> aie;
  // Run up to completion prog on all the tiles
  aie.run<prog>();
}
