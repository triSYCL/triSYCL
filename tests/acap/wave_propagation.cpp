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
    m.w[image_size/3][image_size/2] = 100;
  }

  void compute() {
    auto& m = t::mem();
    for (int j = 0; j < image_size - 1; ++j)
      for (int i = 0; i < image_size - 1; ++i) {
      // grad w
      auto up = m.w[i + 1][j] - m.w[i][j];
      auto vp = m.w[i][j + 1] - m.w[i][j];
      // Integrate speed
      m.u[i][j]  += up*alpha;
      m.v[i][j]  += vp*alpha;
     }
    for (int j = 1; j < image_size; ++j)
      for (int i = 1; i < image_size; ++i) {
      // div speed
      auto wp = (m.u[i][j] - m.u[i - 1][j]) + (m.v[i][j] - m.v[i][j - 1]);
      wp *= m.side[i][j]*(m.depth[i][j] + m.w[i][j]);
      // Integrate depth
      m.w[i][j] += wp;
      // Add some dissipation for the damping
      m.w[i][j] *= 0.995;
    }  }

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
