/* Simple hello world program for ACAP/AI Engine hardware

   NOTE: This requires tweaking to work with the current compiler, for example
   the tile functions relating to reset/etc are pushed down into the API and the
   kernel doesn't need to be commented out. But also some more lower level
   components like the linker script have changed and the main file (most of the
   earlier iterations were a hybrid of Cardano Tool Flow and SYCL Frontend flow)

   RUN: %{execute}%s
*/

#include <SYCL/sycl.hpp>
#include <cstring>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace sycl::vendor::xilinx;

auto size_x = 800;
auto size_y = 600;

uint8_t *input_data;
uint8_t *output_data;

template <typename AIE, int X, int Y> struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  static constexpr unsigned device_tile_start = ((Y & 1) ? 0x29000 : 0x39000);
  auto get_tile_index() { return Y * 50 + X; }
  auto get_tile_size() { return size_x * size_y / 400 * 3; }
  auto get_tile_offset() { return get_tile_index() * get_tile_size(); }
  bool prerun() {
    t::mem_write(acap::aie::xaie::aiev1::args_start,
                 device_tile_start + sizeof(*this));
    t::mem_write(acap::aie::xaie::aiev1::args_start + 4, get_tile_size());

    t::memcpy_h2d(acap::aie::xaie::aiev1::args_start + sizeof(*this),
                  input_data + get_tile_offset(), get_tile_size());
    return true;
  }

  /// The run member function is defined as the tile program
  int8_t *dev_data;
  unsigned size;
  void run() {
    for (unsigned i = 0; i + 3 <= size; i += 3) {
      unsigned mean = ((uint16_t)dev_data[i] + (uint16_t)dev_data[i + 1] +
                       (uint16_t)dev_data[i + 2]) /
                      3;
      dev_data[i] = dev_data[i + 1] = dev_data[i + 2] = mean;
    }
  }

  void postrun() {
    t::memcpy_d2h(output_data + get_tile_offset(),
                  acap::aie::xaie::aiev1::args_start + sizeof(*this),
                  get_tile_size());
  }
};

int main(int argc, char **argv) {

  cv::Mat input = cv::imread("vase2.bmp", cv::IMREAD_COLOR);

  std::cout << "input size = " << input.cols << "x" << input.rows << "x3"
            << std::endl;

  cv::Mat ouput = input.clone();

  auto size_x = input.cols;
  auto size_y = input.rows;

  input_data = input.data;
  output_data = ouput.data;

  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> aie;
  // acap::aie::device<acap::aie::layout::size<1, 1>> aie;
  // Run up to completion of all the tile programs
  aie.run<prog>();

  cv::imwrite("output.bmp", ouput);

  return 0;
}
