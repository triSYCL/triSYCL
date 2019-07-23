/* Testing the AI Engine shim

   RUN: %{execute}%s
*/

#include <SYCL/sycl.hpp>

#include <iostream>

#include <boost/test/minimal.hpp>

using namespace sycl;
using namespace sycl::vendor::xilinx;


/// A small AI Engine program
template <typename AIE, int X, int Y>
struct tile_program : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  void run() {
 }
};


int test_main(int argc, char *argv[]) {
  // Use an empty memory module
  acap::aie::array<acap::aie::layout::full, tile_program> a;
  a.run();

  auto input = a.shim[0].axi_ss
    .input(acap::aie::axi_stream_switch::shim_axi_ss_slave_port::south_0);
  // Just a shortcut for now
  auto output = a.shim[0].axi_ss
    .output(acap::aie::axi_stream_switch::shim_axi_ss_master_port::south_0);
  for (std::uint32_t i = 0; i < 100; ++i) {
    input << i;
    BOOST_CHECK(i == output.read());
  }
  return 0;
}
