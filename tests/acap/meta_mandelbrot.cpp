/* \file

   A generic adaptable library providing a simple DSL for generating
   Mandelbrot-like set for AI Engine

   This shows how it is possible to use functional programming to have
   a generic library taking a host function that can be propagated
   down to the device execution.

   RUN: %{execute}%s
*/

#include <complex>
#include <cmath>
#include <cstdint>
#include <iostream>

#include <sycl/sycl.hpp>
#include "triSYCL/vendor/Xilinx/graphics.hpp"

using namespace sycl::vendor::xilinx;

static auto constexpr image_size = 229;
graphics::application a;

/// All the memory modules are the same
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  // The local pixel tile inside the complex plane
  std::uint8_t plane[image_size][image_size];
};


/** All the tiles run the same Mandelbrot program

    \param[in] F is the function type defining the series to be used
    in the computation
*/
template <typename F, typename AIE, int X, int Y>
struct mandelbrot : acap::aie::tile<AIE, X, Y> {
  using t = mandelbrot;
  // Computation rectangle in the complex plane
  static auto constexpr x0 = -5.0, y0 = -5.0, x1 = 5.0, y1 = 5.0;
  static auto constexpr D = 100; // Divergence norm
  // Size of an image tile
  static auto constexpr xs = (x1 - x0)/t::geo::x_size/image_size;
  static auto constexpr ys = (y1 - y0)/t::geo::y_size/image_size;

  void run() {
    // Access to its own memory
    auto& m = t::mem();
    /// The function defining the series to compute
    F f;
    while (!a.is_done()) {
      for (int i = 0; i < image_size; ++i)
        for (int k, j = 0; j < image_size; ++j) {
          std::complex c { x0 + xs*(X*image_size + i),
                           y0 + ys*(Y*image_size + j) };
          std::complex z { 0.0 };
          for (k = 0; k <= 255; k++) {
            /// Compute the series from an external function
            z = f(z, c);
            if (norm(z) > D)
              break;
          }
          m.plane[j][i] = k;
        }
      a.update_tile_data_image(t::x, t::y, &m.plane[0][0], 0, 255);
    }
  }
};


/// Metafunction to wrap the function type into a mandelbrot AIE tile
/// type compatible with ACAP++
template <typename F>
struct generic_mandelbrot {
  // Bind the function type to the first parameter of the tile type
  template <typename AIE, int X, int Y>
  using m = mandelbrot<F, AIE, X, Y>;
};


/// Create a Mandelbrot-like AI Engine program given a function to
/// define the series to be computed
auto meta_mandelbrot = [] (auto f) {
  static acap::aie::device<acap::aie::layout::size<8,4>> d;
  return
    d.program<generic_mandelbrot<decltype(f)>::template m, memory>();
};


int main(int argc, char *argv[]) {
  // Use the Mandelbrot DSL with various functions to start a
  // wallpaper business
  auto m = meta_mandelbrot(
     /// The tradition:
     // [] (auto z, auto c) { return z*z + c; }
     /// Let's be creative:
     // [] (auto z, auto c) { return z*z*z + c; }
     // [] (auto z, auto c) { return z*z*z*z + c; }
     // [] (auto z, auto c) { return z*(z*(z*(z - 2.0) +3.0) -1.5) + c; }
     // [] (auto z, auto c) { return z*z*z*z*z + c; }
     // [] (auto z, auto c) { return z*z*z*z*z*z + c; }
     //+ [] (auto z, auto c) { return z*z*z*z*z*z*z*z*z*z*z*z + c; }
     // [] (auto z, auto c) { return z*std::exp(z) + z + c; }
     // [] (auto z, auto c) { return std::sinh(z) + c; }
     [] (auto z, auto c) { return std::cosh(z) + c; }
     //+ [] (auto z, auto c) { return std::cos(z) + c; }
  );

  // Open a graphic view of a ME array
  a.start(argc, argv, decltype(m)::geo::x_size,
          decltype(m)::geo::y_size,
          image_size, image_size, 1);
  a.image_grid().get_palette().set(graphics::palette::rainbow, 100, 2, 0);

  // Launch the AI Engine program
  m.run();
  // Wait for the graphics to stop
  a.wait();
}
