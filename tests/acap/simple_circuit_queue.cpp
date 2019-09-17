/* Experimenting with simple network connections using circuit
   switching with own device and queue

   RUN: %{execute}%s
*/

#include <SYCL/sycl.hpp>

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
        auto v = t::template in<char>(0).read();
        // Output = input*2
        t::template out<int>(0) << 2*v;
      }
      // tile(2,0) read from port 1 and write to port 0
      else if constexpr (X == 2 && Y == 0) {
        auto v = t::template in<int>(1).read();
        // Output = input + 1.5
        t::template out<float>(0) << v + 1.5f;
      }
    }
  }
};


int test_main(int argc, char *argv[]) {
  try {
    acap::aie::device<acap::aie::layout::size<3,4>> d;
    d.queue().submit<comm>().wait();

#if 0
  // Connect port 1 of shim(0) to port 0 of tile(1,2) with a "char" link
  d.connect<char>(port::shim { 0, 1 }, port::tile { 1, 2, 0 });
  // Connect port 0 of tile(1,2) to port 1 of tile(2,0) with a "int" link
  d.connect<int>(port::tile { 1, 2, 0 }, port::tile { 2, 0, 1 });
  // Connect port 0 of tile(2,0) to port 0 of shim(1) with a "float" link
  d.connect<float>(port::tile { 2, 0, 0 }, port::shim { 1, 0 });

  // Use the connection from the CPU directly by using the AXI MM to the tile
  aie.tile(1, 2).out<int>(0) << 3;
  // Check we read the correct value on tile(2,0) port 1
  BOOST_CHECK(aie.tile(2, 0).in<int>(1).read() == 3);

  // Try a shim & tile connection directly from the host
  aie.shim(0).bli_out<1, char>() << 42;
  // Check we read the correct value on tile(1,2) port 0
  BOOST_CHECK(aie.tile(1, 2).in<char>(0).read() == 42);

  // Launch the AIE
  auto acap = std::async(std::launch::async, [&] { aie.run(); });
  // Launch a CPU producer
  auto producer = std::async(std::launch::async, [&] {
      for (int i = 0; i < size; ++i)
        // Use the AXI-MM access to the shim registers to send a value
        // into the AXI-SS from the host
        aie.shim(0).bli_out<1, char>() << i;
    });
  // Launch a CPU consumer
  auto consumer = std::async(std::launch::async, [&] {
      float f;
      for (int i = 0; i < size; ++i) {
        aie.shim(1).bli_in<0, float>() >> f;
        // Check we read the correct value
        BOOST_CHECK(f == 2*i + 1.5);
      }
    });

  // Wait for everybody to finish
  acap.get();
  producer.get();
  consumer.get();
#endif

  } catch (sycl::exception &e) {
    // Display the string message of any SYCL exception
    std::cerr << e.what() << std::endl;
    throw;
  }
  return 0;
}
