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
constexpr auto transfers = 1'000'000;
constexpr auto size = transfers*sizeof(std::int32_t);

// Test neighbor communication from tile(0,0) to tile(1,0)
template <typename AIE, int X, int Y>
struct neighbor : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < size; ++i) {
      if constexpr (X == 0 && Y == 0)
        // tile(0,0) write to port 1
        t::out(1) << i;
      else if constexpr (X == 1 && Y == 0) {
        // tile(1,0) read from port 0
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
    using d_t = acap::aie::device<acap::aie::layout::size<2,1>>;
    d_t d;
    // Test neighbor core connection
    d.tile(0,0).connect(d_t::csp::me_1, d_t::cmp::east_0);
    d.tile(1,0).connect(d_t::csp::west_0, d_t::cmp::me_0);
    measure_bandwidth(size, [&] { d.run<neighbor>(); });
  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
