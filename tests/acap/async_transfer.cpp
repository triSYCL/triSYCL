/* AIE NoC benchmark

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <SYCL/sycl.hpp>

#include <chrono>
#include <iostream>

#include <boost/test/minimal.hpp>





#include <boost/fiber/all.hpp>




// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Actual transfers to be done on each tile
// \todo do not use a global variable... Need a lambda API
auto local_transfers = 0;

// Number of values to transfer
constexpr auto right_transfers = 100'000;
#if 0
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
#endif

// Test neighbor communication from each tile to its right neighbor
template <typename AIE, int X, int Y>
struct right_neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    unsigned int src[1];
    std::iota(std::begin(src), std::end(src), 0);
    if constexpr (!t::is_east_column())
      // There is a neighbor on the right: send some data
      t::tx_dma(0).send(src);
    unsigned int dst[1];
    if constexpr (!t::is_west_column())
      // There is a neighbor on the left: receive some data
      t::rx_dma(0).receive(dst);
    if constexpr (!t::is_east_column())
      t::tx_dma(0).wait();
    if constexpr (!t::is_west_column()) {
      t::rx_dma(0).wait();
      // Once it is received, we can check the result
      BOOST_CHECK(ranges::equal(src, dst));
    }
std::cerr << "Sleeping" << std::endl;
boost::this_fiber::sleep_for(std::chrono::seconds { 1 });
  }
};


auto measure_bandwidth = [] (auto transfered_bytes, auto some_work) {
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
//    using d_t = acap::aie::device<layout::size<4,4>>;
    using d_t = acap::aie::device<layout::size<2,1>>;
    d_t d;
    // Configure the AIE NoC connections
    d.for_each_tile_index([&] (auto x, auto y) {
        // When it is possble, connect each tile to its right neighbor
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
      local_transfers = right_transfers/d_t::geo::x_max/d_t::geo::y_size;
      // Only x_max since the last column has no right neigbors to talk to
      auto transmitted_bytes =
        sizeof(std::int32_t)*local_transfers*d_t::geo::x_max*d_t::geo::y_size;

      measure_bandwidth(transmitted_bytes,
                        [&] { d.run<right_neighbor>(); });
  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
