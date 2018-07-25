/* Demo of wave propagation for MathEngine

   Recycle my MINES ParisTech ISIA hands-on

   RUN: %{execute}%s
*/

#include <cstdint>
#include <iostream>

#include <CL/sycl.hpp>

using namespace cl::sycl::vendor::xilinx;

static auto constexpr K = 1/300.0;
static auto constexpr g = 9.81;
static auto constexpr alpha = K*g;
static auto constexpr image_size = 229;
std::unique_ptr<graphics::app> a;

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
    auto& m = t::mem();
    for (int j = 0; j < image_size - 1; ++j)
      for (int i = 0; i < image_size - 1; ++i) {
        // grad w
        auto up = m.w[j][i + 1] - m.w[j][i];
        auto vp = m.w[j + 1][i] - m.w[j][i];
        // Integrate speed
        m.u[j][i] += up*alpha;
        m.v[j][i] += vp*alpha;
      }
    for (int j = 1; j < image_size; ++j)
      for (int i = 1; i < image_size; ++i) {
        // div speed
        auto wp = (m.u[j][i] - m.u[j][i - 1]) + (m.v[j][i] - m.v[j - 1][i]);
        wp *= m.side[j][i]*(m.depth[j][i] + m.w[j][i]);
        // Integrate depth
        m.w[j][i] += wp;
        // Add some dissipation for the damping
        m.w[j][i] *= 0.995;
      }
    if constexpr (t::is_memory_module_up()) {
      auto& above = t::mem_up();
      for (int i = 0; i < image_size; ++i)
        above.w[0][i] = m.w[image_size - 1][i];
    }
    if constexpr ((Y & 1) && t::is_memory_module_right()) {
      auto& right = t::mem_right();
      for (int j = 0; j < image_size; ++j)
        right.w[j][0] = m.w[j][image_size - 1];
    }
    if constexpr (!(Y & 1) && t::is_memory_module_left()) {
      auto& left = t::mem_left();
      for (int j = 0; j < image_size; ++j)
        m.w[j][0] = left.w[j][image_size - 1];
    }
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
  acap::me::array<acap::me::layout::small, tile, memory> me;

  a.reset(new graphics::app { argc, argv, decltype(me)::geo::x_size,
                              decltype(me)::geo::y_size,
                              image_size, image_size, 1 });

  // Launch the MathEngine program
  me.run();
  // Wait for the graphics to stop
  a->wait();
}
