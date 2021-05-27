/* Mandelbrot set for AI Engine as uniform lambda

   RUN: %{execute}%s
*/

#include <complex>
#include <cstdint>
#include <sycl/sycl.hpp>
#include "triSYCL/vendor/Xilinx/graphics.hpp"

using namespace sycl::vendor::xilinx;
auto constexpr image_size = 229;

int main(int argc, char* argv[]) {
  acap::aie::device<acap::aie::layout::size<2, 3>> aie;
  graphics::application a;

  // Open a graphic view of a ME array
  a.start(argc, argv, aie.x_size, aie.y_size, image_size, image_size, 1)
      .image_grid()
      .get_palette()
      .set(graphics::palette::rainbow, 100, 2, 0);

  // Launch the AI Engine tiles
  aie.uniform_run([&](auto th) {
    // The local pixel tile inside the complex plane
    std::uint8_t plane[image_size][image_size];
    // Computation rectangle in the complex plane
    auto constexpr x0 = -2.1, y0 = -1.2, x1 = 0.6, y1 = 1.2;
    auto constexpr D = 100; // Divergence norm
    // Size of an image tile
    auto constexpr xs = (x1 - x0) / th.x_size() / image_size;
    auto constexpr ys = (y1 - y0) / th.y_size() / image_size;
    while (!a.is_done()) {
      for (int j = 0; j < image_size; ++j)
        for (int k, i = 0; i < image_size; ++i) {
          std::complex c { x0 + xs * (th.x() * image_size + i),
                           y0 + ys * (th.y() * image_size + j) };
          std::complex z { 0.0 };
          for (k = 0; norm(z = z * z + c) < D && k <= 255; k++)
            ;
          plane[j][i] = k;
        }
      // \todo something like th.update_tile_data_image(&plane[0][0], 0, 255); ?
      a.update_tile_data_image(th.x(), th.y(), &plane[0][0], 0, 255);
    }
  });
}
