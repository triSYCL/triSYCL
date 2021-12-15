/* Simple hello world program for ACAP/AI Engine with a different
   lambda on each tile

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

int main() {
  /* Define AIE CGRA with all the tiles of a VC1902 & run up to
     completion prog on all the tiles.

     The run function will instantiate a different lambda for each
     tile so the device compiler is executed for every tile, since
     each tile has a different code according to the use of constexpr
     th tile_handler member usage
  */
  acap::aie::device<acap::aie::layout::size<1, 1>> {}.run([](auto& th) {
    std::cout << "Hello, I am the AIE tile (" << th.x_coord() << ',' << th.y_coord() << ")"
              << std::endl;
  });
}
