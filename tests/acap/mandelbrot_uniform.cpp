// Mandelbrot set for AI Engine as uniform lambda
// REQUIRES: acap

// This test uses an old API
// XFAIL

// RUN: %acap_clang %s -o %s.bin
// RUN: %add_acap_result %s.bin
// RUN: rm %s.bin

#include "triSYCL/vendor/Xilinx/graphics.hpp"
#include <complex>
#include <cstdint>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;
auto constexpr image_size = 64;

graphics::application<uint8_t> a;

int main(int argc, char* argv[]) {
  acap::aie::device<acap::aie::layout::size<50, 8>> aie;

  a.set_device(aie);
  // Open a graphic view of a AIE array
  a.start(argc, argv, aie.x_size, aie.y_size, image_size, image_size, 1)
      .image_grid()
      .get_palette()
      .set(graphics::palette::rainbow, 100, 2, 0);

  // Launch the AI Engine tiles
  aie.uniform_run([=](auto& th) {
    // The local pixel tile inside the complex plane
    std::uint8_t plane[image_size][image_size];
    // Computation rectangle in the complex plane
    auto constexpr x0 = -2.1, y0 = -1.2, x1 = 0.6, y1 = 1.2;
    auto constexpr D = 100; // Divergence norm
    // Size of an image tile
    auto xs = (x1 - x0) / th.x_size() / image_size;
    auto ys = (y1 - y0) / th.y_size() / image_size;
    while (!a.is_done()) {
      for (int j = 0; j < image_size; ++j)
        for (int k, i = 0; i < image_size; ++i) {
          std::complex c { x0 + xs * (th.x_coord() * image_size + i),
                           y0 + ys * (th.y_coord() * image_size + j) };
          std::complex z { 0.0 };
          for (k = 0; norm(z = z * z + c) < D && k <= 255; k++)
            ;
          plane[j][i] = k;
        }
      // \todo something like th.update_tile_data_image(&plane[0][0], 0, 255); ?
      a.update_tile_data_image(th.x_coord(), th.y_coord(), &plane[0][0], 0, 255);
    }
  });
}

// notable issues:
//
//  - the graphics::application cannot be a local variable because if
//  graphics::application is a local variable it needs to be captured by the
//  lambda. kernel lambda need to capture by value ([=]) but this causes a copy.
//  and on the host graphics::application contain std::thread which aren't
//  copyable. This was fixed by making graphics::application a global variable.
//
//  - xs and ys cannot be constexpr because th is not known at compile time.
//  that said xs and ys will be constant folded.
