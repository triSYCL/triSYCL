#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>

#include <experimental/mdspan>

#include "triSYCL/vendor/Xilinx/graphics.hpp"
#include <sycl/sycl.hpp>

// Some headers used when debugging
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include <boost/thread.hpp>

using namespace sycl::vendor::xilinx;

using layout = acap::aie::layout::size<5, 5>;
using geography = acap::aie::geography<layout>;

using data_type = double;

auto constexpr image_size = 20;
auto constexpr zoom = 5;

graphics::application<data_type> a;

data_type pure_function(int x, int y, int frame) {
  constexpr int frame_shift = 16;
  constexpr int y_shift = 8;
  return (x & 0xff) + ((y & 0xff) << y_shift) + ((frame & 0xff) << frame_shift);
};

/// A sequential reference implementation of wave propagation
template <auto size_x, auto size_y, auto display_tile_size>
struct reference_impl {
  using space = std::experimental::mdspan<data_type, size_y, size_x>;
  static auto constexpr linear_size = size_x * size_y;
  data_type data_m[linear_size];
  space data{data_m};

  uint32_t frame_id = 0;

  /// Compute a time-step of wave propagation
  void compute() {
    for (int j = 0; j < size_y; ++j)
      for (int i = 0; i < size_x; ++i) {
        data(j, i) = pure_function(j, i, frame_id);
      }
    frame_id++;
  }

  void validate() {
    for (int j = 0; j < size_y / display_tile_size; ++j)
      for (int i = 0; i < size_x / display_tile_size; ++i) {
        auto sp = std::experimental::subspan(
            data,
            std::make_pair(j * display_tile_size, (j + 1) * display_tile_size),
            std::make_pair(i * display_tile_size, (i + 1) * display_tile_size));
        a.validate_tile_data_image(i, j, sp, 0, 200000);
      }
  }

  /// Run the wave propagation
  void run() {
    /// Loop on simulated time
    while (!a.is_done()) {
      compute();
      validate();
    }
  }
};

reference_impl<image_size * acap::aie::geography<layout>::x_size,
               image_size * acap::aie::geography<layout>::y_size, image_size>
    seq;

/// All the memory modules are the same
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  data_type data[image_size][image_size];
};

/// All the tiles run the same program
template <typename AIE, int X, int Y> struct tile : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  uint32_t frame_id;
  void compute() {
    auto &m = t::mem();
    for (int j = 0; j < image_size; ++j)
      for (int i = 0; i < image_size; ++i) {
        m.data[j][i] =
            pure_function(j + (image_size * Y), i + (image_size * X), frame_id);
      }
    frame_id++;
  }

  void display() {
    auto &m = t::mem();
    a.update_tile_data_image(t::x, t::y, &m.data[0][0], 0, 200000);
  }

  void run() {
    frame_id = 0;
    while (!a.is_done()) {
      compute();
      display();
    }
  }
};

int main(int argc, char *argv[]) {
  // An ACAP version of the wave propagation
  acap::aie::device<layout> d;

  a.set_device(d);
#ifdef __SYCL_XILINX_AIE__
  a.enable_data_validation();
#endif
  a.start(argc, argv, decltype(d)::geo::x_size, decltype(d)::geo::y_size,
          image_size, image_size, zoom);
  // Clip the level 127 which is the 0 level of the simulation
  a.image_grid().get_palette().set(graphics::palette::rainbow, 150, 2, 127);

#ifdef __SYCL_XILINX_AIE__
  a.start_data_validation([&]() { seq.run(); });
#endif
  // Launch the AI Engine program
  d.run<tile, memory>();
  // Wait for the graphics to stop
  // a.wait();
}
