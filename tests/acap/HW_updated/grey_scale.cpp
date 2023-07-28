/* Grey scale example on 400 acap cores.
*/

#include <sycl/sycl.hpp>
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
  auto get_tile_index() { return Y * 50 + X; }
  auto get_tile_size() { return size_x * size_y / 400 * 3; }
  auto get_tile_offset() { return get_tile_index() * get_tile_size(); }
  bool prerun() {
    /// acap::hw::args_begin_offset is the start of a region of memory
    /// that is suitable for parmeter passing
    /// acap::hw::self_tile_addr<X, Y> is the start of the tile from the
    /// from the device perspective.
    /// the start of the tile from the host's perspective is 0

    /// The code below is assuming that sizeof(*this) on the host is >= to
    /// sizeof(*this) on the device. this is should always true since
    /// the only different I know is that pointer a 32bits in the device instead
    /// of 64bits on the host

    /// setup int8_t *dev_data to point just after the this object on device.
    t::mem_write(acap::hw::args_begin_offset, acap::hw::self_tile_addr<X, Y> +
                                             acap::hw::args_begin_offset +
                                             sizeof(*this));

    /// setup unsigned size;
    t::mem_write(acap::hw::args_begin_offset + 4, get_tile_size());

    /// copies input_data to where dev_data is pointing on device.
    t::memcpy_h2d(acap::hw::args_begin_offset + sizeof(*this),
                  input_data + get_tile_offset(), get_tile_size());
    return 1;
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

    /// copies the data pointed by dev_data on device to output_data on the host.
    t::memcpy_d2h(output_data + get_tile_offset(),
                  acap::hw::args_begin_offset + sizeof(*this), get_tile_size());
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
  // Run up to completion of all the tile programs
  aie.run<prog>();

  cv::imwrite("output.bmp", ouput);

  return 0;
}
