/* Testing the cascade stream

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>

using namespace cl::sycl;
using namespace cl::sycl::vendor::xilinx;


/// A small MathEngine program
template <typename ME_Array, int X, int Y>
struct tile_program : acap::me::tile<ME_Array, X, Y> {
  using t = acap::me::tile<ME_Array, X, Y>;

  void run() {
    // Do not read at the start of the cascade
    if constexpr (!t::is_cascade_start()) {
      auto cs_in = this->template get_cascade_stream_in<int>();
      auto v = cs_in.read();
      std::cout << "< Tile(" << X << ',' << Y << ") is reading "
                << v << std::endl;
    }
    // Do not write at the end of the cascade
    if constexpr (!t::is_cascade_end()) {
      auto cs_out = t::template get_cascade_stream_out<int>();
      auto v = t::x*1000 + t::y;
      std::cout << "> Tile(" << X << ',' << Y << ") is writing "
                << v << std::endl;
      cs_out << v;
    }
  }
};


int main() {
  // Use an empty memory module
  acap::me::array<acap::me::layout::full, tile_program>{}.run();
}
