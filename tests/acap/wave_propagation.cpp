/* Demo of wave propagation for MathEngine

   Recycle my MINES ParisTech ISIA hands-on

   https://en.wikipedia.org/wiki/Boussinesq_approximation_(water_waves)
   Joseph Valentin Boussinesq, 1872

   RUN: %{execute}%s
*/

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iostream>

//#include <array_ref>
#include <mdspan>

#include <chrono>
#include <thread>
using namespace std::chrono_literals;


#include <CL/sycl.hpp>

using namespace cl::sycl::vendor::xilinx;

static auto constexpr K = 1/300.0;
static auto constexpr g = 9.81;
static auto constexpr alpha = K*g;
static auto constexpr image_size = 229;
std::unique_ptr<graphics::app> a;


/// A sequential reference implementation of wave propagation
template <auto size_x, auto size_y, auto display_tile_size>
struct reference_wave_propagation {
  using space =
    std::experimental::mdspan<double, std::experimental::extents<size_y,
                                                                 size_x>>;
  // It would be nice to have a constexpr static member to express this,
  // but right now size() is a member function
  //double u_m[space::size()];
  static auto constexpr linear_size = size_x*size_y;
  double u_m[linear_size];
  space u { u_m }; // Horizontal speed
  double v_m[linear_size];
  space v { v_m }; // Vertical speed
  double w_m[linear_size];
  space w { w_m }; // Local delta depth
  double side_m[linear_size];
  space side { side_m }; // Hard wall limit
  double depth_m[linear_size];
  space depth { depth_m }; // Average depth

  reference_wave_propagation() {
    for (int j = 0; j < size_y; ++j)
      for (int i = 0; i < size_x; ++i) {
        // No u[j][i] syntax too like in Boost.Multi_Array ?
        u(j,i) = v(j,i) = w(j,i) = 0;
        side(j,i) = K;
        depth(j,i) = 2600.0;
      }
    w(size_y/3,size_x/2+size_x/4) = 100;
  }


  void compute() {
    for (int j = 0; j < size_y - 1; ++j)
      for (int i = 0; i < size_x - 1; ++i) {
        // grad w
        auto up = w(j,i + 1) - w(j,i);
        auto vp = w(j + 1,i)- w(j,i);
        // Integrate speed
        u(j,i) += up*alpha;
        v(j,i) += vp*alpha;
      }
    for (int j = 1; j < size_y; ++j)
      for (int i = 1; i < size_x; ++i) {
        // div speed
        auto wp = (u(j,i) - u(j,i - 1)) + (v(j,i) - v(j - 1,i));
        wp *= side(j,i)*(depth(j,i) + w(j,i));
        // Integrate depth
        w(j,i) += wp;
        // Add some dissipation for the damping
        w(j,i) *= 0.999;
      }
  }

  void run() {
    while (!a->is_done()) {
      compute();
    for (int j = 0; j < size_y/display_tile_size; ++j)
      for (int i = 0; i <  size_x/display_tile_size; ++i) {
        // Should we have w.data() or std::begin(w) instead of &w(0,0) ?
        a->update_tile_data_image(i, j,
                                  std::experimental::subspan
                                  (w,
                                   std::make_pair(j*display_tile_size,
                                                  display_tile_size),
                                   std::make_pair(i*display_tile_size,
                                                  display_tile_size)),
                                  -1.0, 1.0);
      }
    }
  }
};


// All the memory modules are the same
template <typename ME_Array, int X, int Y>
struct memory : acap::me::memory<ME_Array, X, Y> {
  // The local pixel buffer
  double u[image_size][image_size];
  double v[image_size][image_size];
  double w[image_size][image_size]; // Local delta depth
  double side[image_size][image_size];
  double depth[image_size][image_size]; // Average depth
};


static auto minmax_element(const double value[image_size][image_size]) {
  return std::minmax_element(&value[0][0],
                             &value[image_size][image_size]);
}

// All the tiles run the same program
template <typename ME_Array, int X, int Y>
struct tile : acap::me::tile<ME_Array, X, Y> {
  using t = acap::me::tile<ME_Array, X, Y>;

  void initialize_space() {
    auto& m = t::mem();
    for (int j = 0; j < image_size; ++j)
      for (int i = 0; i < image_size; ++i) {
        m.u[j][i] = m.v[j][i] = m.w[j][i] = 0;
        m.side[j][i] = K;
        m.depth[j][i] = 2600.0;
      }
    if (X == 0 && Y == 0) m.w[image_size/3][image_size/2+image_size/4] = 100;
  }

  void compute() {
std::this_thread::sleep_for(50ms);
    auto& m = t::mem();
    if constexpr (!t::is_right_column()) {
      m.lu.locks[10].acquire_with_value(false);
    }
    for (int j = 0; j < image_size - 1; ++j)
      for (int i = 0; i < image_size - 1; ++i) {
        // grad w
        auto up = m.w[j][i + 1] - m.w[j][i];
        auto vp = m.w[j + 1][i] - m.w[j][i];
        // Integrate speed
        m.u[j][i] += up*alpha;
        m.v[j][i] += vp*alpha;
      }
std::this_thread::sleep_for(50ms);

//sleep(1);
    // Transfer first line of u to next memory module on the left
    if constexpr (Y & 1) {
      if constexpr (t::is_memory_module_right()) {
        auto& right = t::mem_right();
        right.lu.locks[2].acquire_with_value(false);
        for (int j = 0; j < image_size; ++j)
          m.u[j][image_size - 1] = right.u[j][0];
        right.lu.locks[2].release_with_value(true);
      }
      if constexpr (!t::is_left_column()) {
        m.lu.locks[2].acquire_with_value(true);
        m.lu.locks[2].release_with_value(false);
      }
    }
    if constexpr (!(Y & 1)) {
      if constexpr (t::is_memory_module_left()) {
        auto& left = t::mem_left();
        left.lu.locks[10].acquire_with_value(true);
        left.lu.locks[2].acquire_with_value(false);
        for (int j = 0; j < image_size; ++j)
          left.u[j][image_size - 1] = m.u[j][0];
        left.lu.locks[2].release_with_value(true);
        left.lu.locks[10].release_with_value(false);
      }
      if constexpr (!t::is_right_column()) {
        m.lu.locks[10].release_with_value(true);
        m.lu.locks[2].acquire_with_value(true);
        m.lu.locks[2].release_with_value(false);
        m.lu.locks[10].acquire_with_value(false);
        m.lu.locks[10].release_with_value(false);
      }
    }
    if constexpr (t::is_memory_module_down()) {
      auto& below = t::mem_down();
      below.lu.locks[3].acquire_with_value(false);
      for (int i = 0; i < image_size; ++i)
        below.v[image_size - 1][i] = m.v[0][i];
      below.lu.locks[3].release_with_value(true);
    }
    if constexpr (!t::is_top_row()) {
      m.lu.locks[3].acquire_with_value(true);
      m.lu.locks[3].release_with_value(false);
    }
std::this_thread::sleep_for(50ms);
    for (int j = 1; j < image_size; ++j)
      for (int i = 1; i < image_size; ++i) {
        // div speed
        auto wp = (m.u[j][i] - m.u[j][i - 1]) + (m.v[j][i] - m.v[j - 1][i]);
        wp *= m.side[j][i]*(m.depth[j][i] + m.w[j][i]);
        // Integrate depth
        m.w[j][i] += wp;
        // Add some dissipation for the damping
        m.w[j][i] *= 0.999;
      }
std::this_thread::sleep_for(50ms);
    if constexpr (t::is_memory_module_up()) {
      auto& above = t::mem_up();
      above.lu.locks[0].acquire_with_value(false);
      for (int i = 0; i < image_size; ++i)
        above.w[0][i] = m.w[image_size - 1][i];
      above.lu.locks[0].release_with_value(true);
    }
    if constexpr (t::is_memory_module_down()) {
      m.lu.locks[0].acquire_with_value(true);
      m.lu.locks[0].release_with_value(false);
    }
    // Transfer last line of w to next memory module on the right
    if constexpr (Y & 1) {
      if constexpr (t::is_memory_module_right()) {
        auto& right = t::mem_right();
        right.lu.locks[1].acquire_with_value(false);
        for (int j = 0; j < image_size; ++j)
          right.w[j][0] = m.w[j][image_size - 1];
        right.lu.locks[1].release_with_value(true);
      }
      if constexpr (!t::is_left_column()) {
        m.lu.locks[1].acquire_with_value(true);
        m.lu.locks[1].release_with_value(false);
      }
    }
    if constexpr (!(Y & 1)) {
      if constexpr (t::is_memory_module_left()) {
        auto& left = t::mem_left();
        left.lu.locks[1].acquire_with_value(false);
        for (int j = 0; j < image_size; ++j)
          m.w[j][0] = left.w[j][image_size - 1];
        left.lu.locks[1].release_with_value(true);
      }
      if constexpr (!t::is_right_column()) {
        m.lu.locks[1].acquire_with_value(true);
        m.lu.locks[1].release_with_value(false);
      }
    }
    static int iteration = 0;
    auto [min_element, max_element] = minmax_element(m.w);
    std::cout << "compute(" << X << ',' << Y
              << ") iteration " << ++iteration << " done"
              << *min_element << ',' << *max_element
              << std::endl;
  }

  void run() {
    initialize_space();
    auto& m = t::mem();
    while (!a->is_done()) {
      compute();
      a->update_tile_data_image(t::x, t::y, &m.w[0][0], -1.0, 1.0);
    }
  }
};

int main(int argc, char *argv[]) {
  acap::me::array<acap::me::layout::size<2,1>, tile, memory> me;

  a.reset(new graphics::app { argc, argv, decltype(me)::geo::x_size,
                              decltype(me)::geo::y_size,
                              image_size, image_size, 1 });

  // Run a sequential reference implementation
  reference_wave_propagation<image_size*decltype(me)::geo::x_size,
                             image_size*decltype(me)::geo::y_size,
                             image_size>{}.run();

  // Launch the MathEngine program
  me.run();
  // Wait for the graphics to stop
  a->wait();
}
