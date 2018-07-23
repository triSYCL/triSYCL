/* Testing the minimal empty MathEngine Tile program and Memory Module

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace cl::sycl::vendor::xilinx;

int test_main(int argc, char *argv[]) {
  /* Test the empty tile program & empty memory module for various
     machine sizes */
  acap::me::array<acap::me::layout::one_pe>{}.run();
  acap::me::array<acap::me::layout::small>{}.run();
  acap::me::array<acap::me::layout::full>{}.run();

  return 0;
}
