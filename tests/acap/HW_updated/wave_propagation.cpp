/* Demo of wave propagation for AI Engine

   Simulation with a conic drop, a circle shoal and a square harbor.

   Recycle MINES ParisTech/ISIA/Telecom Bretagne MSc hands-on HPC labs
   from Ronan Keryell

   https://en.wikipedia.org/wiki/Boussinesq_approximation_(water_waves)
   Joseph Valentin Boussinesq, 1872

   RUN: %{execute}%s
*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>

#include <experimental/mdspan>

#include "triSYCL/vendor/Xilinx/graphics.hpp"
#include <sycl/sycl.hpp>

// #define FULL_DISPLAY

// Some headers used when debugging
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include <boost/thread.hpp>

using namespace sycl::vendor::xilinx;

// The size of the machine to use
// using layout = acap::aie::layout::size<5,4>;
// For a 1920x1080 display
// using layout = acap::aie::layout::size<18,8>;
using layout = acap::aie::layout::size<1, 1>;
// For a 3440x1440 display
// using layout = acap::aie::layout::size<33,12>;
using geography = acap::aie::geography<layout>;
boost::barrier cpu_barrier{geography::size};

using data_type = float;
using idx_type = int;

data_type constexpr K = 1.0 / 300.0;
data_type constexpr g = 9.81;
data_type constexpr alpha = K * g;
data_type constexpr damping = 0.999;

idx_type constexpr image_size = 20;
idx_type constexpr no_halo_size = image_size - 1;
idx_type constexpr last_image_idx = image_size - 1;

data_type constexpr max_value = 30.0;
data_type constexpr min_value = -30.0;

idx_type constexpr zoom = 5;
/// Add a drop almost between tile (1,1) and (2,2)
idx_type constexpr x_drop = image_size * 1 - image_size / 2 - 1;
idx_type constexpr y_drop = image_size * 1 - image_size / 2 - 1;
data_type constexpr drop_value = 100.0;
data_type constexpr drop_radius = 5.0;

/** Time-step interval between each display.
    Use 1 to display all the frames, 2 for half the frame and so on. */
auto constexpr display_time_step = 2;

graphics::application<data_type> a;

auto epsilon = 0.01;

/// Compute the square power of a value
constexpr auto square = [](auto v) constexpr { return v * v; };

/// Compute the contribution of a drop to the water height
constexpr auto add_a_drop = [](int x, int y) constexpr -> data_type {
  // return (y & 0xff) | ((x & 0xff) << 8);
  // The square radius to the drop center
  data_type r = square(x - x_drop) + square(y - y_drop);
  // A cone of height drop_value centered on the drop center
  return r < square(drop_radius)
             ? drop_value * (square(drop_radius) - r) / square(drop_radius)
             : 0;
};

/// Add a circular shoal in the water with half the depth
constexpr auto shoal_factor = [](auto x, auto y) constexpr -> data_type {
  /// The shoal center coordinates
  idx_type constexpr x_shoal = image_size * 8 - 3;
  idx_type constexpr y_shoal = image_size * 4;
  data_type constexpr shoal_radius = 200.0;
  data_type constexpr shoal_val = 2600.0;

  // The square radius to the shoal center
  data_type r = square(x - x_shoal) + square(y - y_shoal);
  // A disk centered on the shoal center
  return r < square(shoal_radius) ? 0.5 : 1;
};

/// Add a square harbor in the water
constexpr auto is_harbor = [](auto x, auto y) constexpr -> bool {
  /// The square harbor center coordinates
  auto constexpr x_harbor = image_size * 2 - image_size / 3;
  auto constexpr y_harbor = image_size * 2 - image_size / 3;
  auto constexpr length_harbor = image_size;

  // A square centered on the harbor center
  auto harbor =
      x_harbor - length_harbor / 2 <= x && x <= x_harbor + length_harbor / 2 &&
      y_harbor - length_harbor / 2 <= y && y <= y_harbor + length_harbor / 2;
  // Add also a breakwater below
  auto constexpr width_breakwater = image_size / 3;
  auto breakwater = x_harbor <= x && x <= x_harbor + width_breakwater &&
                    y < y_harbor - image_size
                    // Add some 4-pixel holes every image_size/2
                    && (y / 4) % (image_size / 8);
  return harbor || breakwater;
};

/// All the memory modules are the same
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  data_type u[image_size][image_size];     //< Horizontal speed
  data_type v[image_size][image_size];     //< Vertical speed
  data_type w[image_size][image_size];     //< Local delta depth
  data_type side[image_size][image_size];  //< Hard wall limit
  data_type depth[image_size][image_size]; //< Average depth
};

/// All the tiles run the same program
template <typename AIE, int X, int Y> struct tile : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  void initialize_space() {
    auto &m = t::mem();
    for (int j = 0; j < image_size; ++j)
      for (int i = 0; i < image_size; ++i) {
        m.u[j][i] = m.v[j][i] = 0;
        m.side[j][i] = K * (!is_harbor(i + no_halo_size * X,
                                       j + no_halo_size * Y));
        // m.side[j][i] = K;
        m.depth[j][i] = shoal_factor(i + no_halo_size * X,
                                              j + no_halo_size * Y);
        // m.depth[j][i] = 1.0;
        // Add a drop using the global coordinate taking into account the halo
        m.w[j][i] =
            add_a_drop(i + no_halo_size * X, j + no_halo_size * Y);
      }
  }

  void compute() {
    auto &m = t::mem();

    for (int j = 0; j < image_size; ++j)
      for (int i = 0; i < last_image_idx; ++i) {
        // dw/dx
        auto north = m.w[j][i + 1] - m.w[j][i];

        // Integrate horizontal speed
        m.u[j][i] += north * alpha;
      }

    for (int j = 0; j < last_image_idx; ++j)
      for (int i = 0; i < image_size; ++i) {
        // dw/dy
        auto vs = m.w[j + 1][i] - m.w[j][i];
        // Integrate vertical speed
        m.v[j][i] += vs * alpha;
      }

#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    cpu_barrier.wait();
#endif
    // Transfer first column of u to next memory module to the West
    if constexpr (Y & 1) {
      if constexpr (t::is_memory_module_east()) {
        auto& east = t::mem_east();
        for (int j = 0; j < image_size; ++j)
          m.u[j][last_image_idx] = east.u[j][0];
      }
    }
    if constexpr (!(Y & 1)) {
      if constexpr (t::is_memory_module_west()) {
        auto &west = t::mem_west();
        for (int j = 0; j < image_size; ++j)
          west.u[j][last_image_idx] = m.u[j][0];
      }
    }

    if constexpr (t::is_memory_module_south()) {
      auto &below = t::mem_south();
      for (int i = 0; i < image_size; ++i)
        below.v[last_image_idx][i] = m.v[0][i];
    }

#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    cpu_barrier.wait();
#endif
    for (int j = 1; j < image_size; ++j)
      for (int i = 1; i < image_size; ++i) {
        // div speed
        auto wp = (m.u[j][i] - m.u[j][i - 1]) + (m.v[j][i] - m.v[j - 1][i]);
        wp *= m.side[j][i] * (m.depth[j][i] + m.w[j][i]);
        // Integrate depth
        m.w[j][i] += wp;
        // Add some dissipation for the damping
        m.w[j][i] *= damping;
      }

#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    cpu_barrier.wait();
#endif

    if constexpr (t::is_memory_module_north()) {
      auto &above = t::mem_north();
      for (int i = 0; i < image_size; ++i)
        above.w[0][i] = m.w[last_image_idx][i];
    }

#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    cpu_barrier.wait();
#endif

    // Transfer last line of w to next memory module on the East
    if constexpr (Y & 1) {
      if constexpr (t::is_memory_module_east()) {
        auto& east = t::mem_east();
        for (int j = 0; j < image_size; ++j)
          east.w[j][0] = m.w[j][last_image_idx];
      }
    }
    if constexpr (!(Y & 1)) {
      if constexpr (t::is_memory_module_west()) {
        auto &west = t::mem_west();
        for (int j = 0; j < image_size; ++j)
          m.w[j][0] = west.w[j][last_image_idx];
      }
    }

#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    cpu_barrier.wait();
#endif
  }

  void full_display(const char *str) {
#if defined(__SYCL_DEVICE_ONLY__)
    auto &m = t::mem();
    t::log(str);
    t::log("u v w side depth\n");
    t::log((uint32_t)&m.u[0][0]);
    a.update_tile_data_image(t::x, t::y, &m.u[0][0], min_value, max_value);
#else
#endif
  }

  // memory<AIE, X, Y> m;

  volatile data_type frame[no_halo_size][no_halo_size];
  void display() {
    auto &m = t::mem();
    for (int j = 0; j < no_halo_size; ++j)
      for (int i = 0; i < no_halo_size; ++i) {
        frame[j][i] = m.w[j][i];
      }
#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    cpu_barrier.wait();
#endif
    a.update_tile_data_image(t::x, t::y, (data_type*)&frame[0][0], min_value, max_value);
  }

  void run() {
    initialize_space();
    display();
    // Loop on simulated time
    for (int time = 0; !a.is_done(); time++) {
#ifdef __SYCL_XILINX_AIE__
      t::barrier();
#else
      cpu_barrier.wait();
#endif
        compute();
        // Display every display_time_step
        if (time % display_time_step == 0) {
          display();
        }
    }
  }
};


int main(int argc, char *argv[]) {
  // An ACAP version of the wave propagation
  acap::aie::device<layout> d;

  a.set_device(d);
  a.start(argc, argv, decltype(d)::geo::x_size, decltype(d)::geo::y_size,
          no_halo_size, no_halo_size, zoom);
  // Clip the level 127 which is the 0 level of the simulation
  a.image_grid().get_palette().set(graphics::palette::rainbow, 150, 2, 127);

  // Launch the AI Engine program
  d.run<tile, memory>();
  // Wait for the graphics to stop
  // a.wait();
}
