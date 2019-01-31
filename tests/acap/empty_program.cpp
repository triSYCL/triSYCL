/* Testing the minimal empty AI Engine Tile program and Memory Module

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace cl::sycl::vendor::xilinx;

int test_main(int argc, char *argv[]) {
  /* Test the empty tile program & empty memory module for various
     machine sizes */
  acap::aie::array<acap::aie::layout::one_pe>{}.run();
  acap::aie::array<acap::aie::layout::small>{}.run();
  acap::aie::array<acap::aie::layout::full>{}.run();

  return 0;
}
