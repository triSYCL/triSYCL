/* Simple hello world program for ACAP/AI Engine with the same lambda
   on each tile

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

int main() {
  /* Define an AIE CGRA with all the tiles of a VC1902 & run
     collectively up to completion some code on all the tiles.

     The uniform run function will instantiate uniformly the same
     lambda for all the tiles so the tile device compiler is executed
     only once, since each tile has the same code
  */
  acap::aie::device<acap::aie::layout::vc1902> {}.uniform_run([](auto& th) {
    std::cout << "Hello, I am the AIE tile (" << th.x_coord() << ',' << th.y_coord() << ")"
              << std::endl;
  });

  // Try with a mutable lambda
  acap::aie::device<acap::aie::layout::vc1902> {}.uniform_run(
      [v = 0](auto& th) mutable {
        auto old_v = v;
        std::cout << "Again, I am the AIE tile (" << th.x_coord() << ',' << th.y_coord()
                  << ") mutating " << old_v << " into " << ++v << std::endl;
      });
}
