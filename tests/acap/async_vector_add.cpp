/* Strip-mined vector addition distributed on AIE CGRA

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <chrono>
#include <iostream>
#include <string>

// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

using d_t = acap::aie::device<layout::size<8, 4>>;

using value_t = unsigned int;

auto constexpr strip_mine_size = 1280;
auto constexpr strip_mine_factor = 3;
// Pick some length to demo some strip-mining and AIE distribution
auto constexpr vector_length =
    strip_mine_size * strip_mine_factor * d_t::geo::x_size * d_t::geo::y_size;

// Each tile receive 2 small vectors and send back the result
template <typename AIE, int X, int Y>
struct vector_add : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    value_t a_strip[strip_mine_size];
    value_t b_strip[strip_mine_size];
    value_t c_strip[strip_mine_size];
    for (auto sf = 0; sf < strip_mine_factor; ++sf) {
      t::rx_dma(0).receive(a_strip).receive(b_strip).wait();
      for (auto i = 0; i < strip_mine_size; ++i)
        c_strip[i] = a_strip[i] + b_strip[i];
      t::tx_dma(0).send(c_strip).wait();
    }
  }
};

auto measure_bandwidth = [](auto transfered_bytes, const auto& some_work) {
  auto starting_point = clk::now();
  some_work();
  // Get the duration in seconds as a double
  std::chrono::duration<double> duration = clk::now() - starting_point;
  std::cout << " time: " << duration.count()
            << " s, bandwidth: " << transfered_bytes / duration.count()
            << " B/s" << std::endl;
};

int main() try {
  sycl::buffer<value_t> va { vector_length };
  sycl::buffer<value_t> vb { vector_length };
  sycl::buffer<value_t> vc { vector_length };
  {
    auto a = va.get_access<sycl::access::mode::write>();
    std::iota(a.begin(), a.end(), 0);
  }
  {
    auto b = vb.get_access<sycl::access::mode::write>();
    std::iota(b.begin(), b.end(), vector_length);
  }

  d_t d;
  // Configure the AIE NoC connections
  d.for_each_tile_index([&](auto x, auto y) {
    d.tile(x, y).connect(d_t::cass::s_south_range[y], d_t::cmp::dma_0);
    d.tile(x, y).connect(d_t::csp::dma_0, d_t::cass::m_south_range[y]);
    for (auto i = y + 1; i < d_t::geo::y_size; ++i) {
      d.tile(x, y).connect(d_t::cass::s_south_range[i],
                           d_t::cass::m_north_range[i]);
      d.tile(x, y).connect(d_t::cass::s_north_range[i],
                           d_t::cass::m_south_range[i]);
    }
  });
  d.for_each_tile_x_index([&](auto x) {
    for (auto y = 0; y < d_t::geo::y_size; ++y) {
      d.shim(x).connect(d_t::sass::s_south_range[y],
                        d_t::sass::m_north_range[y]);
      d.shim(x).connect(d_t::sass::s_north_range[y],
                        d_t::sass::m_south_range[y]);
    }
  });

  /* Some (future) FPGA kernel reading the input vectors and streaming
     them to the AIE */
  sycl::queue {}.submit([&](auto& h) {
    auto a = va.get_access<sycl::access::mode::read>();
    auto b = vb.get_access<sycl::access::mode::read>();
    h.single_task([=, &d] {
      for (auto sf = 0; sf < strip_mine_factor; ++sf)
        for (auto x = 0; x < d_t::geo::x_size; ++x)
          for (auto y = 0; y < d_t::geo::y_size; ++y) {
            for (auto i = 0; i < strip_mine_size; ++i)
              d.shim(x).bli_out(y)
                  << a[((sf * d_t::geo::x_size + x) * d_t::geo::y_size + y) *
                           strip_mine_size +
                       i];
            for (auto i = 0; i < strip_mine_size; ++i)
              d.shim(x).bli_out(y)
                  << b[((sf * d_t::geo::x_size + x) * d_t::geo::y_size + y) *
                           strip_mine_size +
                       i];
          }
    });
  });

  /* Some (future) FPGA kernel streaming the output vectors from the
     AIE and writing back the vector */
  sycl::queue {}.submit([&](auto& h) {
    auto c = vc.get_access<sycl::access::mode::discard_write>();
    h.single_task([=, &d] {
      for (auto sf = 0; sf < strip_mine_factor; ++sf)
        d.for_each_tile_index([&](auto x, auto y) {
          for (auto i = 0; i < strip_mine_size; ++i)
            c[((sf * d_t::geo::x_size + x) * d_t::geo::y_size + y) *
                  strip_mine_size +
              i] = d.shim(x).bli_in(y).read();
        });
    });
  });

  auto bytes_transfered = va.get_size() + vb.get_size() + vc.get_size();
  measure_bandwidth(bytes_transfered, [&] { d.run<vector_add>(); });

  // Dump the network configuration
  d.display("async_vector_add.tex");

  {
    auto a = va.get_access<sycl::access::mode::read>();
    auto b = vb.get_access<sycl::access::mode::read>();
    auto c = vc.get_access<sycl::access::mode::read>();
    for (auto i = 0; i < va.get_count(); ++i)
      if (a[i] + b[i] != c[i]) {
        std::cerr << "Error at i = " << i << " for a = " << a[i]
                  << ", b = " << b[i] << ", c = " << c[i] << std::endl;
        std::terminate();
      }
  }
} catch (std::exception& e) {
  // Display the string message of any SYCL exception
  std::cerr << e.what() << std::endl;
}
