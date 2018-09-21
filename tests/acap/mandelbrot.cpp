/* Mandelbrot set for MathEngine

   RUN: %{execute}%s
*/

#include <complex>
#include <cstdint>
#include <iostream>

#include <CL/sycl.hpp>

using namespace cl::sycl::vendor::xilinx;

static auto constexpr image_size = 229;
std::unique_ptr<graphics::app> a;

// All the memory modules are the same
template <typename ME_Array, int X, int Y>
struct memory : acap::me::memory<ME_Array, X, Y> {
  // The local pixel plane
  std::uint8_t plane[image_size][image_size];
};

// All the tiles run the same Mandelbrot program
template <typename ME_Array, int X, int Y>
struct mandelbrot : acap::me::tile<ME_Array, X, Y> {
  using t = acap::me::tile<ME_Array, X, Y>;
  // Computation rectangle in the complex plane
  static auto constexpr x0 = -2.0, y0 = -1.0, x1 = 1.0, y1 = 1.0;
  static auto constexpr M = 100;
  // Size of an image tile
  static auto constexpr xs = (x1 - x0)/t::geo::x_size/image_size;
  static auto constexpr ys = (y1 - y0)/t::geo::y_size/image_size;

  void run() {
    // Access to its own memory
    auto& m = t::mem();
    for (int i = 0; i < image_size; ++i)
      for (int k, j = 0; j < image_size; ++j) {
        std::complex c { x0 + xs*(X*image_size + i),
                         y0 + ys*(Y*image_size + j) };
        std::complex z { 0.0 };
        for (k = 0; k <= 255; k++) {
          z = z*z + c;
          if (norm(z) > M)
            break;
        }
        m.plane[j][i] = k;
      }
    a->update_tile_data_image(t::x, t::y, &m.plane[0][0], 0, 255);
  }
};

int main(int argc, char *argv[]) {
  acap::me::array<acap::me::layout::size<2,3>, mandelbrot, memory> me;
  // Open a graphic view of a ME array
  a.reset(new graphics::app { argc, argv, decltype(me)::geo::x_size,
                              decltype(me)::geo::y_size,
                              image_size, image_size, 1 });

  // Launch the MathEngine program
  me.run();
  // Wait for the graphics to stop
  a->wait();
}
