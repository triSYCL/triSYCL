/* Mandelbrot set for MathEngine

   RUN: %{execute}%s
*/

#include <complex>
#include <cstdint>
#include <iostream>

#include <CL/sycl.hpp>

using namespace std::chrono_literals;
using namespace cl::sycl::vendor::xilinx;

static auto constexpr image_size = 229;
std::unique_ptr<graphics::app> a;

// By default all the tiles have an empty program
template <typename ME_Array, int X, int Y>
struct tile : acap::me::tile<ME_Array, X, Y> {
  using t = acap::me::tile<ME_Array, X, Y>;

  static auto constexpr x0 = -2.0, y0 = -1.0, x1 = 1.0, y1 = 1.0;
  static auto constexpr M = 100;

  static auto constexpr xs = (x1 - x0)/t::geo::x_size/image_size;
  static auto constexpr ys = (y1 - y0)/t::geo::y_size/image_size;

  void run() {
    std::uint8_t plane[image_size][image_size];
    for (int i = 0; i < image_size; ++i)
      for (int k, j = 0; j < image_size; ++j) {
        std::complex c { x0 + xs*(X*image_size + i),
                         y0 + ys*(Y*image_size + j) };
        std::complex z { 0.0 };
        for(k = 0; k <= 255; k++) {
          z = z*z + c;
          if (norm(z) > M)
            break;
        }
        plane[j][i] = k;
      }
    a->update_tile_data_image(t::x, t::y, &plane[0][0], 0, 255);
  }
};

int main(int argc, char *argv[]) {
  acap::me::array<acap::me::layout::small, tile> me;

  a.reset(new graphics::app { argc, argv, decltype(me)::geo::x_size,
                              decltype(me)::geo::y_size,
                              image_size, image_size, 1 });

  // Launch the MathEngine program
  me.run();
  // Wait for the graphics to stop
  a->wait();
}
