/* Simple hello world program launched per tile individually

   RUN: %{execute}%s
*/
//#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <iostream>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

int main() {
  // Define an AIE CGRA with all the tiles of a VC1902
  using d_t = acap::aie::device<acap::aie::layout::vc1902>;
  d_t d;
  //  Submit some work on each tile, which is SYCL sub-device
  d.for_each_tile([](auto& t) {
    /* This will instantiate uniformly the same
       lambda for all the tiles so the tile device compiler is executed
       only once, since each tile has the same code
    */
    t.single_task([&](auto& th) {
      std::cout << "Hello, I am the AIE tile (" << th.x() << ',' << th.y()
                << ")" << std::endl;
    });
  });
  // Wait for the end of each tile execution
  d.for_each_tile([](auto& t) { t.wait(); });
}
