/* AIE AXI stream FIFO test of each tile

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <chrono>
#include <iostream>

#include <boost/test/minimal.hpp>

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Number of test iterations
constexpr auto number_of_tests = 100;

// Each tile just writes and reads data on a FIFO loop-back
template <typename AIE, int X, int Y>
struct fifo_loopback : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  // Since we have 2 FIFO on the path, we can enqueue twice the FIFO
  // capacity without blocking
  static constexpr auto fifo_path_capacity =
    t::geo::core_axi_stream_switch::fifo_depth*2;

  void run() {
    for (int count = 0; count < number_of_tests; ++count) {
      // Make all the write first to detect any case of deadlock
      for (int i = 0; i < fifo_path_capacity; ++i)
        t::out(0) << i;
      int receive;
      for (int i = 0; i < fifo_path_capacity; ++i) {
        t::in(0) >> receive;
        BOOST_CHECK(receive == i);
      }
    }
  }
};

int test_main(int argc, char *argv[]) {
  try {
    using d_t = acap::aie::device<layout::size<4,4>>;
    d_t d;
    // Configure the AIE AXI stream switch to have a loop-back through
    // the FIFOs on each tile
    d.for_each_tile([&] (auto& t) {
      // Connect the AIE core tile output 0 to FIFO 0 input
      t.connect(d_t::csp::me_0, d_t::cmp::fifo_0);
      // Connect the tile FIFO 0 output to FIFO 1 input
      t.connect(d_t::csp::fifo_0, d_t::cmp::fifo_1);
      // Connect the tile FIFO 1 output to AIE core tile input
      t.connect(d_t::csp::fifo_1, d_t::cmp::me_0);
    });

    d.run<fifo_loopback>();
  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
