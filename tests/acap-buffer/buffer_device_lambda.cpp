/* Create 1 buffer per tile and work on it with an accessor

   RUN: %{execute}%s
*/
//#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <iostream>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

int main() {
  // Define an AIE CGRA with all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> d;
  // 1 buffer per tile
  sycl::buffer<int> b[d.x_size][d.y_size];
  // Initialize on the host each buffer with 3 sequential values
  d.for_each_tile_index([&](int x, int y) {
    b[x][y] = { 3 };
    sycl::host_accessor a { b[x][y] };
    std::iota(a.begin(), a.end(), (d.x_size * y + x) * a.get_count());
  });
  //  Submit some work on each tile, which is SYCL sub-device
  d.for_each_tile_index([&](int x, int y) {
    d.tile(x, y).submit([&](auto& cgh) {
      acap::aie::accessor a { b[x][y], cgh };
      cgh.single_task([=] {
        for (auto& e : a)
          e += 42;
      });
    });
  });
  // Wait for the end of each tile execution
  d.for_each_tile([](auto& t) { t.wait(); });
  // Check the result
  d.for_each_tile_index([&](int x, int y) {
    for (sycl::host_accessor a { b[x][y] };
         auto&& [i, e] : ranges::views::enumerate(a))
      if (e != (d.x_size * y + x) * a.get_count() + i + 42)
        throw "Bad computation";
  });
}
