/* Demo of wave propagation for MathEngine

   Recycle my MINES ParisTech ISIA hands-on

   https://en.wikipedia.org/wiki/Boussinesq_approximation_(water_waves)
   Joseph Valentin Boussinesq, 1872

   RUN: %{execute}%s
*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>

#include <mdspan>

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include <boost/thread.hpp>

#include <CL/sycl.hpp>

using namespace cl::sycl::vendor::xilinx;
namespace fundamentals_v3 = std::experimental::fundamentals_v3;

// The size of the machine to use
//using layout = acap::me::layout::size<5,4>;
using layout = acap::me::layout::size<2,1>;
using geography = acap::me::geography<layout>;
boost::barrier b1 { geography::size };
boost::barrier b2 { geography::size };
boost::barrier b3 { geography::size };
boost::barrier b4 { geography::size };



static auto constexpr K = 1/300.0;
static auto constexpr g = 9.81;
static auto constexpr alpha = K*g;
static auto constexpr damping = 0.999;

#if 0
static auto constexpr image_size = 100;
static auto constexpr x_drop = 90;
static auto constexpr y_drop = 7;
static auto constexpr drop_value = 100;
#else
static auto constexpr image_size = 3;
static auto constexpr x_drop = 1;
static auto constexpr y_drop = 1;
static auto constexpr drop_value = 10;
#endif

graphics::application a;

auto epsilon = 0.01;

/** Compare the values of 2 2D mdspan of the same geometry

    Display any discrepancy between an acap and reference mdspan
*/
auto compare_2D_mdspan = [](auto message, const auto &acap, const auto &ref) {
  assert(acap.extent(0) == ref.extent(0));
  assert(acap.extent(1) == ref.extent(1));
  for (int j = 0; j < acap.extent(0); ++j)
    for (int i = 0; i < acap.extent(1); ++i)
      if (std::abs(acap(j,i) - ref(j,i)) > epsilon) {
        TRISYCL_DUMP_T(std::dec << '\t' << message
                       << " acap(" << j << ',' << i << ") = " << acap(j,i)
                       << "  ref(" << j << ',' << i << ") = " << ref(j,i));
      }
};

/// A sequential reference implementation of wave propagation
template <auto size_x, auto size_y, auto display_tile_size>
struct reference_wave_propagation {
  using space = fundamentals_v3::mdspan<double, size_y, size_x>;
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
  // For debug
  double up_m[linear_size];
  space up { up_m };
  double vp_m[linear_size];
  space vp { vp_m };
  double wp_m[linear_size];
  space wp { wp_m };

  /// Initialize the state variables
  reference_wave_propagation() {
    for (int j = 0; j < size_y; ++j)
      for (int i = 0; i < size_x; ++i) {
        // No u[j][i] syntax too like in Boost.Multi_Array ?
        u(j,i) = v(j,i) = w(j,i) = 0;
        side(j,i) = K;
        depth(j,i) = 2600.0;
      }
//    w(size_y/3,size_x/2+size_x/4) = 100;
    w(y_drop,x_drop+(image_size-1)) = drop_value;
//    if (X == 0 && Y == 0) m.w[image_size/3][image_size/2+image_size/4] = 100;
  }


  /// Compute a time-step of wave propagation
  void compute() {
    for (int j = 0; j < size_y; ++j)
      for (int i = 0; i < size_x - 1; ++i) {
        // dw/dx
        up(j,i) = w(j,i + 1) - w(j,i);
        // Integrate horizontal speed
        u(j,i) += up(j,i)*alpha;
      }
    for (int j = 0; j < size_y - 1; ++j)
      for (int i = 0; i < size_x; ++i) {
        // dw/dy
        vp(j,i) = w(j + 1,i) - w(j,i);
        // Integrate vertical speed
        v(j,i) += vp(j,i)*alpha;
      }
    for (int j = 1; j < size_y; ++j)
      for (int i = 1; i < size_x; ++i) {
        // div speed
        wp(j,i) = (u(j,i) - u(j,i - 1)) + (v(j,i) - v(j - 1,i));
        wp(j,i) *= side(j,i)*(depth(j,i) + w(j,i));
        // Integrate depth
        w(j,i) += wp(j,i);
        // Add some dissipation for the damping
        w(j,i) *= damping;
        if (i == 2 & j == 1) {
          std::cerr << "w(" << j << "," << i << ") = "
                    << w(j,i) << std::endl;
        }
      }
  }


  /// Run the wave propagation
  void run() {
    /// Loop on simulated time
    while (!a.is_done()) {
      compute();
      for (int j = 0; j < size_y/display_tile_size; ++j)
        for (int i = 0; i <  size_x/display_tile_size; ++i) {
          /* Split the data in sub-windows with a subspan

             Display actually one redundant line/column on each
             bottom/left to mimic the halo in the ACAP case
          */
          auto sp = fundamentals_v3::subspan
            (w,
             std::make_pair(j*display_tile_size,
                            1 + (j + 1)*display_tile_size),
             std::make_pair(i*display_tile_size,
                            1 + (i + 1)*display_tile_size));
          a.update_tile_data_image(i, j, sp, -1.0, 1.0);
        }
    }
  }

  template <typename Array, typename MDspan_ref>
  void compare_with_sequential_reference_e(const char *message, int x, int y,
                                           Array &arr,
                                           const MDspan_ref &ref) {
    const fundamentals_v3::mdspan<double, image_size, image_size> md
    { &arr[0][0] };

    // Take into account 1 line/column of overlapping halo
    int x_offset = md.extent(1) - 1;
    int y_offset = md.extent(0) - 1;
    auto mdref = fundamentals_v3::subspan(ref,
                                          std::make_pair(y*y_offset,
                                                         1 + (y + 1)*y_offset),
                                          std::make_pair(x*x_offset,
                                                         1 + (x + 1)*x_offset));
    compare_2D_mdspan(message, md, mdref);
  }


  /* The global time of the simulation

     Do not put it inside compare_with_sequential_reference because,
     since it is templated, there is then an instance per tile and the
     chaos happens
  */
  static inline int global_time = 0;
  static inline std::mutex protect_time;

  template <typename Mem>
  void compare_with_sequential_reference(int time, int x, int y, Mem &m) {
    {
      std::lock_guard lg { protect_time };
      TRISYCL_DUMP_T(std::dec << "TILE(" << x << ',' << y << ") Time local: "
                     << time << ", global: " << global_time);
      if (global_time != time) {
        /* Advance the sequential computation by one step so that we
           can do the comparison */
        compute();
        ++global_time;
      }
      compare_with_sequential_reference_e("w", x, y, m.w, w);
      compare_with_sequential_reference_e("u", x, y, m.u, u);
      compare_with_sequential_reference_e("v", x, y, m.v, v);
      compare_with_sequential_reference_e("wp", x, y, m.wp, wp);
      compare_with_sequential_reference_e("up", x, y, m.up, up);
      compare_with_sequential_reference_e("vp", x, y, m.vp, vp);
    }
  }
};


// A sequential reference implementation of the wave propagation
reference_wave_propagation
<(image_size - 1)*acap::me::geography<layout>::x_size + 1,
 (image_size - 1)*acap::me::geography<layout>::y_size + 1,
 image_size - 1> seq;


// All the memory modules are the same
template <typename ME_Array, int X, int Y>
struct memory : acap::me::memory<ME_Array, X, Y> {
  // The local pixel buffer
  double u[image_size][image_size];
  double v[image_size][image_size];
  double w[image_size][image_size]; // Local delta depth
  double side[image_size][image_size];
  double depth[image_size][image_size]; // Average depth
  // For debug
  double up[image_size][image_size];
  double vp[image_size][image_size];
  double wp[image_size][image_size];

};


TRISYCL_DEBUG_ONLY(
static auto minmax_element(const double value[image_size][image_size]) {
  return std::minmax_element(&value[0][0],
                             &value[image_size][image_size]);
}
)

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
    if (X == 1 && Y == 0)
      m.w[y_drop][x_drop] = drop_value;
  }

  void compute() {
    b1.wait();

    auto& m = t::mem();

    for (int j = 0; j < image_size; ++j)
      for (int i = 0; i < image_size - 1; ++i) {
        // dw/dx
        m.up[j][i] = m.w[j][i + 1] - m.w[j][i];
        // Integrate horizontal speed
        m.u[j][i] += m.up[j][i]*alpha;
      }

    for (int j = 0; j < image_size - 1; ++j)
      for (int i = 0; i < image_size; ++i) {
        // dw/dy
        m.up[j][i] = m.w[j][i + 1] - m.w[j][i];
        m.vp[j][i] = m.w[j + 1][i] - m.w[j][i];
        // Integrate vertical speed
        m.v[j][i] += m.vp[j][i]*alpha;
      }

    b2.wait();

    // Transfer first column of u to next memory module on the left
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
        left.lu.locks[2].acquire_with_value(false);
        for (int j = 0; j < image_size; ++j)
          left.u[j][image_size - 1] = m.u[j][0];
        left.lu.locks[2].release_with_value(true);
      }
      if constexpr (!t::is_right_column()) {
        m.lu.locks[2].acquire_with_value(true);
        m.lu.locks[2].release_with_value(false);
      }
    }

    b3.wait();

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

    b4.wait();

    for (int j = 1; j < image_size; ++j)
      for (int i = 1; i < image_size; ++i) {
        // div speed
        m.wp[j][i] = (m.u[j][i] - m.u[j][i - 1]) + (m.v[j][i] - m.v[j - 1][i]);
        m.wp[j][i] *= m.side[j][i]*(m.depth[j][i] + m.w[j][i]);
        // Integrate depth
        m.w[j][i] += m.wp[j][i];
        // Add some dissipation for the damping
        m.w[j][i] *= damping;
        if (X == 0 && i == 2 & j == 1) {
          std::cerr << "m.w[" << j << "][" << i << "] = "
                    << m.w[j][i] << std::endl;
          //std::cerr << "t::is_memory_module_right() right.u[" << j << "][0] = "
          //<< right.u[j][0] << std::endl;
        }
      }

    b1.wait();

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

    b2.wait();

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

    b3.wait();

    TRISYCL_DEBUG_ONLY(static int iteration = 0;
                       auto [min_element, max_element] = minmax_element(m.w);)
    TRISYCL_DUMP_T(std::dec << "compute(" << X << ',' << Y
                   << ") iteration " << ++iteration << " done, min = "
                   << *min_element << ", max = " << *max_element);
  }

  void run() {
    initialize_space();
    auto& m = t::mem();
    fundamentals_v3::mdspan<double, image_size, image_size> md { &m.w[0][0] };
    /// Loop on simulated time
    for (int time = 0; !a.is_done(); ++time) {
      // Uncomment this to have sequential cosimulation and comparison
      seq.compare_with_sequential_reference(time, t::x, t::y, m);
      compute();
      a.update_tile_data_image(t::x, t::y, md, -1.0, 1.0);
    }
  }
};


int main(int argc, char *argv[]) {
  // An ACAP version of the wave propagation
  acap::me::array<layout, tile, memory> me;

  a.start(argc, argv, decltype(me)::geo::x_size,
          decltype(me)::geo::y_size,
          image_size, image_size, 20);
#if 0
  // Run the sequential reference implementation
  seq.run();
#endif
  // Launch the MathEngine program
  me.run();
  // Wait for the graphics to stop
  a.wait();
}
