/* Testing the cascade stream in AI Engine

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>

using namespace cl::sycl;
using namespace cl::sycl::vendor::xilinx;


/// A small MathEngine program
template <typename AIE, int X, int Y>
struct tile_program : acap::aie::tile<AIE, X, Y> {
  // Get tile information through this shortcut
  using t = acap::aie::tile<AIE, X, Y>;

  void run() {
    // Do not read at the start of the cascade
    if constexpr (!t::is_cascade_start()) {
      // Need template here because of... pure C++
      auto cs_in = t::template get_cascade_stream_in<int>();
      auto v = cs_in.read();
      std::cout << "< Tile(" << X << ',' << Y << ") is reading "
                << v << std::endl;
    }
    // Do not write at the end of the cascade
    if constexpr (!t::is_cascade_end()) {
      // Need template here because of... pure C++. Can use also this->
      auto cs_out = this->template get_cascade_stream_out<int>();
      auto v = t::x*1000 + t::y;
      std::cout << "> Tile(" << X << ',' << Y << ") is writing "
                << v << std::endl;
      cs_out << v;
    }
  }
};


int main() {
  // Use an empty memory module
  acap::aie::array<acap::aie::layout::full, tile_program>{}.run();
}