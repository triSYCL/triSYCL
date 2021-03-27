/* AIE NoC benchmark

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <chrono>
#include <iostream>

#include <boost/test/minimal.hpp>

#include <boost/fiber/all.hpp>

// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Actual transfers to be done on each tile to be computed before running
auto local_transfers = -1;

// Number of values to transfer
constexpr auto data_size = 100;

// Total data communication
constexpr auto total_data_communication = 10'000;

/* Future API example
// Test neighbor communication from each tile to its right neighbor
template <typename AIE, int X, int Y>
struct right_neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < local_transfers; ++i) {
      if constexpr (!t::is_east_column()) {
        // There is a neighbor on the right: send some data
        t::tx_dma(0).write(42);
        t::tx_dma(0).locker(3).acquire_when(0).write(i).release_with(1);
      }
      if constexpr (!t::is_west_column()) {
        // There is a neighbor on the left: receive some data
        int receive_1, receive_2;
        t::rx_dma(0).read(receive_1);
        t::rx_dma(0).locker(2).acquire_when(0).read(receive_2).release_with(1);
      }
      if constexpr (!t::is_east_column())
        t::locker(3).acquire_when(1).release_with(0);
      if constexpr (!t::is_west_column())
        t::locker(2).acquire_when(1).release_with(0);
    }
  }
};
*/

// Test neighbor communication from each tile to its right neighbor
template <typename AIE, int X, int Y>
struct right_neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    unsigned int src[data_size];
    unsigned int dst[data_size];
    std::iota(std::begin(src), std::end(src), 0);
    for (int i = 0; i < local_transfers; ++i) {
      if constexpr (!t::is_east_column())
        // There is a neighbor on the right: send some data
        t::tx_dma(0).send(src);
      if constexpr (!t::is_west_column())
        // There is a neighbor on the left: receive some data
        t::rx_dma(0).receive(dst);
      if constexpr (!t::is_east_column())
        // There is a neighbor on the right: wait for the end of transmission
        t::tx_dma(0).wait();
      if constexpr (!t::is_west_column())
        // There is a neighbor on the left: wait for the end of reception
        t::rx_dma(0).wait();
    }
    if constexpr (!t::is_west_column())
      // Once it is received, we can check the result
      BOOST_CHECK(ranges::equal(src, dst));
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

int test_main(int argc, char* argv[]) {
  try {
    // using d_t = acap::aie::device<layout::size<2, 1>>;
    using d_t = acap::aie::device<layout::vc1902>;
    d_t d;
    // Configure the AIE NoC connections
    d.for_each_tile_index([&](auto x, auto y) {
      // When it is possible, connect each tile to its right neighbor
      if (d_t::geo::is_x_y_valid(x + 1, y)) {
        d.tile(x, y).connect(d_t::csp::dma_0, d_t::cmp::east_0);
        d.tile(x + 1, y).connect(d_t::csp::west_0, d_t::cmp::dma_0);
      }
    });

    // Dump the configuration
    d.display("async_transfer.tex");

    std::cout << "Start right neighbor with AIE device (" << d_t::geo::x_size
              << ',' << d_t::geo::y_size << ')' << std::endl;

    // Right now the communications are globally costly so keep them
    // globally constant
    local_transfers =
        total_data_communication / (d_t::geo::x_max - 1) / d_t::geo::y_size;
    // Only x_max since the last column has no right neigbors to talk to
    auto transmitted_bytes = sizeof(std::int32_t) * data_size *
                             local_transfers * (d_t::geo::x_max - 1) *
                             d_t::geo::y_size;

    measure_bandwidth(transmitted_bytes, [&] { d.run<right_neighbor>(); });
  } catch (sycl::exception& e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    // Rethrow to make clear something bad happened
    throw;
  }
  return 0;
}
