/* Simple hello world program launched per tile individually

   RUN: %{execute}%s
*/

#include <iostream>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

int main() {
  // Define an AIE CGRA with all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> d;
  // Submit some work on each tile, which is SYCL sub-device
  d.for_each_tile([] (auto& t) {
                    t.submit([] (auto &h) {
                            std::cout << "Hello, I am the AIE tile (" << h.x()
                                      << ','  << h.y()  << ")" << std::endl;
                          });
                    });
  // Wait for the end of each tile execution
  d.for_each_tile([] (auto& t) { t.wait(); });
}
