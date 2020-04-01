/* Experimenting with simple network connections using circuit
   switching with own device and queue

   RUN: %{execute}%s
*/

#include <SYCL/sycl.hpp>

#include <chrono>
#include <iostream>

#include <boost/test/minimal.hpp>

// Use precise time measurement
using clk = std::chrono::high_resolution_clock;

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Number of values to transfer
constexpr auto transfers = 100'000;
constexpr auto size = transfers*sizeof(std::int32_t);

// Test neighbor communication from tile(0,0) to tile(1,0)
template <typename AIE, int X, int Y>
struct right_neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < size; ++i) {
      if constexpr (!t::is_right_column())
        // There is a neighbor on the right: send some data
        t::out(1) << i;
      if constexpr (!t::is_left_column()) {
        // There is a neighbor on the left: receive some data
        int receive;
        t::in(0) >> receive;
      }
    }
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
    // Run on various sizes
    auto sizes = boost::hana::make_tuple(layout::size<2,1> {},
                                         layout::size<2,2> {},
                                         layout::size<4,4> {},
                                         layout::vc1902 {});
    boost::hana::for_each(sizes, [&] (auto s) {
      using d_t = acap::aie::device<decltype(s)>;
      d_t d;
      // Test neighbor core connection.
      // Configure the AIE NoC connections
      d.for_each_tile_index([&] (auto x, auto y) {
        // When it is possble, connect each tile to its right neighbor
        if (d_t::geo::is_x_y_valid(x + 1, y)) {
          d.tile(x, y).connect(d_t::csp::me_1, d_t::cmp::east_0);
          d.tile(x + 1, y).connect(d_t::csp::west_0, d_t::cmp::me_0);
        }
      });
      std::cout << "Start with AIE device (" << d_t::geo::x_size
                << ',' << d_t::geo::y_size << ')' << std::endl;
      // Only x_max since the last column has no right neigbors to talk to
      auto transmitted_bytes =
        sizeof(std::int32_t)*transfers*d_t::geo::x_max*d_t::geo::y_size;
      measure_bandwidth(transmitted_bytes,
                        [&] { d.template run<right_neighbor>(); });
    });
  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
