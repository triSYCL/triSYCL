/* Experimenting with simple network connections using circuit
   switching with own device and queue

   RUN: %{execute}%s
*/

// Put the tile code on fiber too to boost the performances
#define TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER 1

#include <sycl/sycl.hpp>

#include <future>
#include <iostream>

#include <boost/test/minimal.hpp>

using namespace sycl::vendor::xilinx;
using namespace sycl::vendor::xilinx::acap::aie;

// Number of values to transfer
constexpr auto size = 10;

/// A small AI Engine program
template <typename AIE, int X, int Y>
struct comm : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < size; ++i) {
      // tile(1,2) read from port 0 and write to port 0
      if constexpr (X == 1 && Y == 2) {
        auto v = t::in(0).read();
        // Output = input*2
        t::out(0) << 2*v;
      }
      // tile(2,0) read from port 1 and write to port 0
      else if constexpr (X == 2 && Y == 0) {
        auto v = t::in(1).read();
        // Output = input + 1
        t::out(0) << v + 1;
      }
    }
  }
};


int test_main(int argc, char *argv[]) {
  try {
    acap::aie::device<acap::aie::layout::size<3,4>> d;

    // Test the communication API from the host point-of-view

    /*
      Some syntax ideas...
      aie.connect<int>({ 1, 2 }, { 2, 3 });
      aie.connect<float>({ 2, 3 }, shim { 1 });
      aie.connect<float>(all, all);
      aie.connect<float>(all, broadcast_line);
      aie.connect<float>(all, broadcast_column);
      aie.connect<float>({ 2, 3 }, broadcast_column);
      aie.connect<float>(line { 2 }, broadcast_column);
      aie.connect<float>(column { 3 }, broadcast_column);
    */

    // Connect port 1 of shim(0) to port 0 of tile(1,2)
    d.connect(port::shim { 0, 1 }, port::tile { 1, 2, 0 });
    // Connect port 0 of tile(1,2) to port 1 of tile(2,0)
    d.connect(port::tile { 1, 2, 0 }, port::tile { 2, 0, 1 });
    // Connect port 0 of tile(2,0) to port 0 of shim(1)
    d.connect(port::tile { 2, 0, 0 }, port::shim { 1, 0 });

    // Use the connection from the CPU directly by using the AXI MM to the tile
    d.tile(1, 2).out(0) << 3;
    // Check we read the correct value on tile(2,0) port 1
    BOOST_CHECK(d.tile(2, 0).in(1).read() == 3);

    // Try a shim & tile connection directly from the host
    d.shim(0).bli_out(1) << 42;
    // Check we read the correct value on tile(1,2) port 0
    BOOST_CHECK(d.tile(1, 2).in(0).read() == 42);

    // Launch the AIE comm program
    auto aie_future = d.queue().submit<comm>();


  // Launch a CPU producer
  auto producer = std::async(std::launch::async, [&] {
      for (int i = 0; i < size; ++i)
        // Use the AXI-MM access to the shim registers to send a value
        // into the AXI-SS from the host
        d.shim(0).bli_out(1) << i;
    });

  // Launch a CPU consumer
  auto consumer = std::async(std::launch::async, [&] {
      float f;
      for (int i = 0; i < size; ++i) {
        d.shim(1).bli_in(0) >> f;
        // Check we read the correct value
        BOOST_CHECK(f == 2*i + 1);
      }
    });

  // Wait for everybody to finish
  producer.get();
  consumer.get();
  aie_future.wait();

  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
