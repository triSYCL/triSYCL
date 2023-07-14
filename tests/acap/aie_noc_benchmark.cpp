/* AIE NoC benchmark

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <chrono>
#include <iostream>

#include <boost/test/minimal.hpp>

// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Actual transfers to be done on each tile
// \todo do not use a global variable... Need a lambda API
auto local_transfers = 0;

// Number of values to transfer
constexpr auto right_transfers = 100'000;

// Test neighbor communication from each tile to its right neighbor
template <typename AIE, int X, int Y>
struct right_neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < local_transfers; ++i) {
      if constexpr (!t::is_east_column())
        // There is a neighbor on the right: send some data
        t::out(0) << i;
      if constexpr (!t::is_west_column()) {
        // There is a neighbor on the left: receive some data
        int receive;
        t::in(0) >> receive;
      }
    }
  }
};

// Number of values to transfer
constexpr auto square_transfers = 100'000;

// Each tile make a square round trip with the neighborhood
template <typename AIE, int X, int Y>
struct square_neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    // Only if there is a square neighborhood towards the South West
    if constexpr (t::is_south_west_valid())
      for (int i = 0; i < local_transfers; ++i) {
        t::out(1) << i;
        int receive;
        t::in(1) >> receive;
      }
  }
};


auto measure_bandwidth = [] (auto transfered_bytes, const auto& some_work) {
  auto starting_point = clk::now();
  some_work();
  // Get the duration in seconds as a double
  std::chrono::duration<double> duration = clk::now() - starting_point;
  std::cout << " time: " << duration.count()
            << " s, bandwidth: " << transfered_bytes/duration.count() << " B/s"
            << std::endl;
};


int test_main(int argc, char *argv[]) {
  try {
    // Run on various sizes
    auto sizes = boost::hana::make_tuple(/*layout::size<2,1> {},
                                         layout::size<2,2> {},*/
                                         layout::size<4,4> {}/*,
                                         layout::vc1902 {}*/);
    boost::hana::for_each(sizes, [&] (auto s) {
      using d_t = acap::aie::device<decltype(s)>;
      d_t d;
      // Test neighbor core connection.
      // Configure the AIE NoC connections
      d.for_each_tile_index([&] (auto x, auto y) {
        // When it is possble, connect each tile to its right neighbor
        if (d_t::geo::is_x_y_valid(x + 1, y)) {
          // Only use _0 ports to avoid conflict with other benchmark
          d.tile(x, y).connect(d_t::csp::me_0, d_t::cmp::east_0);
          d.tile(x + 1, y).connect(d_t::csp::west_0, d_t::cmp::me_0);
        }
      });

      std::cout << "Start right neighbor with AIE device (" << d_t::geo::x_size
                << ',' << d_t::geo::y_size << ')' << std::endl;

      // Right now the communications are globally costly so keep them
      // globally constant
      local_transfers = right_transfers/d_t::geo::x_max/d_t::geo::y_size;
      // Only x_max since the last column has no right neigbors to talk to
      auto transmitted_bytes =
        sizeof(std::int32_t)*local_transfers*d_t::geo::x_max*d_t::geo::y_size;

      measure_bandwidth(transmitted_bytes,
                        [&] { d.template run<right_neighbor>(); });

      // Avoid a division by 0
      if constexpr (d_t::geo::y_max > 0) {
        d.for_each_tile_index([&] (auto x, auto y) {
          // Only if there is a square neighborhood towards the bottom left
          if (d_t::geo::is_x_y_valid(x - 1, y - 1)) {
          // Only use the _1 ports to avoid conflict with other benchmark
            d.tile(x, y).connect(d_t::csp::me_1, d_t::cmp::south_1);
            d.tile(x, y - 1).connect(d_t::csp::north_1, d_t::cmp::west_1);
            d.tile(x - 1, y - 1).connect(d_t::csp::east_1, d_t::cmp::north_1);
            d.tile(x - 1, y).connect(d_t::csp::south_1, d_t::cmp::east_1);
            d.tile(x, y).connect(d_t::csp::west_1, d_t::cmp::me_1);
          }
        });

        std::cout << "Start square neighbor with AIE device ("
                  << d_t::geo::x_size << ',' << d_t::geo::y_size
                  << ')' << std::endl;
        // Right now the communications are globally costly so keep them
        // globally constant
        local_transfers = square_transfers/d_t::geo::x_max/d_t::geo::y_max;
        // Only x_max since the last column has no right neigbors to talk to
        transmitted_bytes =
          sizeof(std::int32_t)*local_transfers*d_t::geo::x_max*d_t::geo::y_max;
        measure_bandwidth(transmitted_bytes,
                          [&] { d.template run<square_neighbor>(); });
        // Dump the configuration in LaTeX
        d.display("aie_noc_benchmark.tex");
      }
    });
  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
