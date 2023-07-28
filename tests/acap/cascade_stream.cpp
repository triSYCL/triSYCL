/* Testing the cascade stream in AI Engine

   RUN: %{execute}%s
*/

#include <sycl/sycl.hpp>

#include <cassert>
#include <iostream>

using namespace sycl;
using namespace sycl::vendor::xilinx;


/// A small AI Engine program
template <typename AIE, int X, int Y>
struct tile_program : acap::aie::tile<AIE, X, Y> {
  // Get tile information through this shortcut
  using t = acap::aie::tile<AIE, X, Y>;

  void run() {
    int v = 0;
    // Do not read at the start of the cascade
    if constexpr (!t::is_cascade_start()) {
      v = t::template cascade_read<int>();
      std::cout << "< Tile(" << X << ',' << Y << ") is reading "
                << v << std::endl;
    }
    // Incrementing the value from 0 along the cascade should give
    // exactly the cascade_linear_id
    assert(v == t::cascade_linear_id());
    ++v;
    // Do not write at the end of the cascade
    if constexpr (!t::is_cascade_end()) {
      std::cout << "> Tile(" << X << ',' << Y << ") is writing "
                << v << std::endl;
      t::cascade_write(v);
    }
  }

  // Some unit tests
  static_assert(X == t::geo::linear_x(t::linear_id()));
  static_assert(Y == t::geo::linear_y(t::linear_id()));
  static_assert(X == t::geo::cascade_linear_x(t::cascade_linear_id()));
  static_assert(Y == t::geo::cascade_linear_y(t::cascade_linear_id()));
};


int main() {
  // Use an empty memory module
  acap::aie::device<acap::aie::layout::full> {}.run<tile_program>();
}
