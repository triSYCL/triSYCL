/* Testing the minimal empty AI Engine Tile program and Memory Module

   RUN: %{execute}%s
*/

#include <sycl/sycl.hpp>

#include <boost/test/minimal.hpp>

using namespace sycl::vendor::xilinx;

int test_main(int argc, char *argv[]) {
  /* Test the empty tile program & empty memory module with 4
     machine sizes & API shortcut */
  acap::aie::device<acap::aie::layout::one_pe>{}.queue().submit<>().wait();
  acap::aie::device<acap::aie::layout::small>{}.queue().run<>();
  acap::aie::device<acap::aie::layout::full>{}.run<>();

  acap::aie::device<acap::aie::layout::vc1902> d;
  acap::aie::program p { d };
  p.run();

  return 0;
}
