/* Demo of wave propagation for AI Engine

   Simulation with a conic drop, a circle shoal and a square harbor.

   Recycle MINES ParisTech/ISIA/Telecom Bretagne MSc hands-on HPC labs
   from Ronan Keryell

   https://en.wikipedia.org/wiki/Boussinesq_approximation_(water_waves)
   Joseph Valentin Boussinesq, 1872

   RUN: %{execute}%s
*/

/** Predicate for time-step comparison with sequential cosimulation

    0: for no co-simulation

    1: compare the parallel execution with sequential execution
*/
#define COMPARE_WITH_SEQUENTIAL_EXECUTION 0

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>

#include <experimental/mdspan>

#include <sycl/sycl.hpp>
#include "triSYCL/vendor/Xilinx/graphics.hpp"

// Some headers used when debugging
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include <boost/thread.hpp>

using namespace sycl::vendor::xilinx;

/// The type used to do all the computations
using data_t = float;

// The size of the machine to use
// using layout = acap::aie::layout::size<5,4>;
// For a 1920x1080 display
using layout = acap::aie::layout::size<18,8>;
// using layout = acap::aie::layout::size<4, 4>;
// For a 3440x1440 display
// using layout = acap::aie::layout::size<33,12>;
using geography = acap::aie::geography<layout>;
boost::barrier cpu_barrier{geography::size};

using data_t = float;
using idx_type = int;

data_t constexpr K = 1.0 / 300.0;
data_t constexpr g = 9.81;
data_t constexpr alpha = K * g;
/// Some dissipation factor to avoid divergence
data_t constexpr damping = 0.999;

idx_type constexpr image_size = 20;
idx_type constexpr no_halo_size = image_size - 1;
idx_type constexpr last_image_idx = image_size - 1;

data_t constexpr max_value = 30.0;
data_t constexpr min_value = -30.0;

idx_type constexpr zoom = 5;
/// Add a drop almost between tile (1,1) and (2,2)
idx_type constexpr x_drop = image_size * 1 - image_size / 2 - 1;
idx_type constexpr y_drop = image_size * 1 - image_size / 2 - 1;
data_t constexpr drop_value = 100.0;
data_t constexpr drop_radius = 5.0;

/** Time-step interval between each display.
    Use 1 to display all the frames, 2 for half the frame and so on. */
auto constexpr display_time_step = 10;

graphics::application<data_t> a;

auto epsilon = 0.01;

#if COMPARE_WITH_SEQUENTIAL_EXECUTION == 1
/** Compare the values of 2 2D mdspan of the same geometry

    Display any discrepancy between an acap and reference mdspan
*/
auto compare_2D_mdspan = [](auto message, const auto &acap, const auto &ref) {
  assert(acap.extent(0) == ref.extent(0));
  assert(acap.extent(1) == ref.extent(1));
  for (int j = 0; j < acap.extent(0); ++j)
    for (int i = 0; i < acap.extent(1); ++i)
      if (std::abs(acap[j, i] - ref[j, i]) > epsilon) {
        TRISYCL_DUMP_T(std::dec << '\t' << message
                       << " acap(" << j << ',' << i << ") = " << acap[j, i]
                       << "  ref(" << j << ',' << i << ") = " << re[j, i]);
      }
};
#endif

/// Compute the square power of a value
constexpr auto square = [](auto v) constexpr { return v * v; };

/// Compute the contribution of a drop to the water height
constexpr auto add_a_drop = [](int x, int y) constexpr -> data_t {
  // The square radius to the drop center
  data_t r = square(x - x_drop) + square(y - y_drop);
  // A cone of height drop_value centered on the drop center
  return r < square(drop_radius)
             ? drop_value * (square(drop_radius) - r) / square(drop_radius)
             : 0;
};

/// Add a circular shoal in the water with half the depth
constexpr auto shoal_factor = [](auto x, auto y) constexpr -> data_t {
  /// The shoal center coordinates
  idx_type constexpr x_shoal = image_size * 8 - 3;
  idx_type constexpr y_shoal = image_size * 4;
  data_t constexpr shoal_radius = 200.0;
  data_t constexpr shoal_val = 2600.0;

  // The square radius to the shoal center
  data_t r = square(x - x_shoal) + square(y - y_shoal);
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

/// A sequential reference implementation of wave propagation
template <auto size_x, auto size_y, auto display_tile_size>
struct reference_wave_propagation {
  using space = std::mdspan<data_t, std::extents<std::size_t, size_y, size_x>>;
  // It would be nice to have a constexpr static member to express this,
  // but right now size() is a member function
  static auto constexpr linear_size = size_x * size_y;
  data_t u_m[linear_size];
  data_t v_m[linear_size];
  data_t w_m[linear_size];
  data_t side_m[linear_size];
  data_t depth_m[linear_size];
  space u{u_m}; // Horizontal speed
  space v{v_m}; // Vertical speed
  space w{w_m}; // Local delta depth
  space side{side_m}; // Hard wall limit
  space depth{depth_m}; // Average depth

  /// Initialize the state variables
  reference_wave_propagation() {
    for (int j = 0; j < size_y; ++j)
      for (int i = 0; i < size_x; ++i) {
        u[j, i] = v[j, i] = w[j, i] = 0;
        side[j, i] = K*(!is_harbor(i, j));
        depth[j, i] = 2600.0*shoal_factor(i, j);
        w[j, i] += add_a_drop(i, j);
      }
  }


  /// Compute a time-step of wave propagation
  void compute() {
    for (int j = 0; j < size_y; ++j)
      for (int i = 0; i < size_x - 1; ++i) {
        // dw/dx
        auto north = w[j, i + 1] - w[j, i];
        // Integrate horizontal speed
        u[j, i] += north*alpha;
      }
    for (int j = 0; j < size_y - 1; ++j)
      for (int i = 0; i < size_x; ++i) {
        // dw/dy
        auto vp = w[j + 1, i] - w[j, i];
        // Integrate vertical speed
        v[j, i] += vp*alpha;
      }
    for (int j = 1; j < size_y; ++j)
      for (int i = 1; i < size_x; ++i) {
        // div speed
        auto wp = (u[j, i] - u[j, i - 1]) + (v[j, i] - v[j - 1, i]);
        wp *= side[j, i]*(depth[j, i] + w[j, i]);
        // Integrate depth
        w[j, i] += wp;
        // Add some dissipation for the damping
        w[j, i] *= damping;
      }
  }

  void validate() {
    for (int j = 0; j < size_y / display_tile_size; ++j)
      for (int i = 0; i < size_x / display_tile_size; ++i) {

        /* Split the data in sub-windows with a subspan

           Display actually one redundant line/column on each
           South/West to mimic the halo in the ACAP case
        */
        auto sp = std::experimental::submdspan(
            w,
            std::make_pair(j * display_tile_size, (j + 1) * display_tile_size),
            std::make_pair(i * display_tile_size, (i + 1) * display_tile_size));
        a.validate_tile_data_image(i, j, sp, min_value, max_value);
      }
  }

  /// Run the wave propagation
  void run() {
    /// Loop on simulated time
    validate();
    int time = 0;
    while (!a.is_done()) {
      compute();
      if (time++ % display_time_step == 0) {
        validate();
      }
    }
  }
};

/** A sequential reference implementation of the wave propagation

    Use no_halo_size for the tile size to skip the halo zone of 1
    pixel in X and Y
*/
reference_wave_propagation<
    no_halo_size * acap::aie::geography<layout>::x_size + 1,
    no_halo_size * acap::aie::geography<layout>::y_size + 1, no_halo_size>
    seq;
// reference_wave_propagation<no_halo_size, no_halo_size, no_halo_size> seq;

/// All the memory modules are the same
template <typename AIE, int X, int Y>
struct memory : acap::aie::memory<AIE, X, Y> {
  data_t u[image_size][image_size];     //< Horizontal speed
  data_t v[image_size][image_size];     //< Vertical speed
  data_t w[image_size][image_size];     //< Local delta depth
  data_t side[image_size][image_size];  //< Hard wall limit
  data_t depth[image_size][image_size]; //< Average depth
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
        m.depth[j][i] = shoal_factor(i + no_halo_size * X,
                                              j + no_halo_size * Y);
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

  volatile data_t frame[no_halo_size][no_halo_size];
  void display() {
    auto &m = t::mem();
    for (int j = 0; j < no_halo_size; ++j)
      for (int i = 0; i < no_halo_size; ++i) {
        frame[j][i] = m.w[j][i];
      }
#ifdef __SYCL_XILINX_AIE__
    t::barrier();
#else
    std::this_thread::sleep_for(150ms);
    cpu_barrier.wait();
#endif
    a.update_tile_data_image(t::x, t::y, (data_t*)&frame[0][0], min_value, max_value);
  }

  void run() {
    initialize_space();
    display();
    // Loop on simulated time
    for (int time = 0; !a.is_done(); time++) {
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
#ifdef __SYCL_XILINX_AIE__
  // a.enable_data_validation();
#endif
  a.start(argc, argv, decltype(d)::geo::x_size, decltype(d)::geo::y_size,
          no_halo_size, no_halo_size, zoom);
  // Clip the level 127 which is the 0 level of the simulation
  a.image_grid().get_palette().set(graphics::palette::rainbow, 150, 2, 127);

#ifdef __SYCL_XILINX_AIE__
  // a.start_data_validation([&]() { seq.run(); });
#endif
  // Launch the AI Engine program
  d.run<tile, memory>();
  // Wait for the graphics to stop
  a.wait();
}
