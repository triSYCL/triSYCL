/* Testing the shim

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;
using namespace cl::sycl::vendor::xilinx;


/// A small MathEngine program
template <typename ME_Array, int X, int Y>
struct tile_program : acap::me::tile<ME_Array, X, Y> {
  using t = acap::me::tile<ME_Array, X, Y>;

  void run() {
 }
};


int test_main(int argc, char *argv[]) {
  // Use an empty memory module
  acap::me::array<acap::me::layout::full, tile_program> a;
  a.run();

  auto input = a.shim[0].axi_ss
    .input(acap::me::axi_stream_switch::shim_axi_ss_slave_port::south_0);
  // Just a shortcut for now
  auto output = a.shim[0].axi_ss
    .output(acap::me::axi_stream_switch::shim_axi_ss_master_port::south_0);
  for (std::uint32_t i = 0; i < 100; ++i) {
    input << i;
    BOOST_CHECK(i == output.read());
  }
  return 0;
}
