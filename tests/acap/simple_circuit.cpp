/* Experimenting with simple network connections using circuit switching

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>
#include <thread>

using namespace cl::sycl::vendor::xilinx;

// Number of values to transfer
constexpr auto size = 10;

/// A small AI Engine program

template <typename AIE, int X, int Y>
struct tile : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  void run() {
    for (int i = 0; i < size; ++i) {
      if constexpr (X == 1 && Y == 2) {
        auto v = 2 /*t::template in<char>(0).read()*/;
        t::template out<int>(0) << 2*v;
      }
      if constexpr (X == 2 && Y == 3) {
        auto v = t::template in<int>(1).read();
std::cout << v << std::endl;
//        t::template out<float>(1) << v + 1.5;
      }
    }
  }
};


int main() {
  acap::aie::array<acap::aie::layout::size<3,4>, tile> aie;
/*
  aie.noc.connect<char>(shim { 0 }, { 2, 3 });
  aie.noc.connect<int>({ 1, 2 }, { 2, 3 });
  aie.noc.connect<float>({ 2, 3 }, shim { 1 });
  aie.noc.connect<float>(all, all);
  aie.noc.connect<float>(all, broadcast_line);
  aie.noc.connect<float>(all, broadcast_column);
  aie.noc.connect<float>({ 2, 3 }, broadcast_column);
  aie.noc.connect<float>(line { 2 }, broadcast_column);
  aie.noc.connect<float>(column { 3 }, broadcast_column);
*/

  // Connect port 0 of tile(1,2) to port 1 of tile(2,0)
  aie.connect<int>({ 1, 2 }, 0, { 2, 0 }, 1);
  //  aie.connect<float>({ 1, 2 }, 0, { 2, 3 }, 1);

  // Use the connection from the CPU directly by using the AXI MM to the tile
  aie.tile(1,2).out<int>(0) << 3;
  std::cout << aie.tile(2, 0).in<int>(1).read() << std::endl;

exit(1);
  // Launch the AIE
  auto acap = std::thread([&] { aie.run(); });
  // Launch a CPU producer
  std::thread producer { [&] {
      for (int i = 0; i < size; ++i)
        // Use the AXI-MM access to the shim registers to send a value
        // into the AXI0S from the host
        /*aie.shim(2).in<char>(0) << i*/;
    }};
  // Launch a CPU consumer
  std::thread consumer { [&] {
      for (int i = 0; i < size; ++i)
        /*aie.shim(1).in<float>(1) >> i*/;
    }};

  // Waut for everybody to finish
  acap.join();
  producer.join();
  consumer.join();
}
