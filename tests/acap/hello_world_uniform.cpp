/* Simple hello world program for ACAP/AI Engine with the same
   callable on each tile

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

struct f {
  int a = 0;
  void operator()(auto& th) {
    ++a;
    std::cout << "Hello, I am the AIE tile (" << th.x_coord() << ',' << th.y_coord() << ")"
              << " a = " << a << std::endl;
  }
};

struct const_f {
  int a = 0;
  void operator()(auto& th) const {
    std::cout << "Hello, I am the AIE const tile (" << th.x_coord() << ',' << th.y_coord()
              << ")" << std::endl;
  }
};

int main() {
  /* Define an AIE CGRA with all the tiles of a VC1902 & run
     collectively up to completion some code on all the tiles.

     The uniform run function will instantiate uniformly the same
     lambda for all the tiles so the tile device compiler is executed
     only once, since each tile has the same code
  */
  acap::aie::device<acap::aie::layout::vc1902> {}.uniform_run(f {});
  acap::aie::device<acap::aie::layout::vc1902> {}.uniform_run(const_f {});
}
