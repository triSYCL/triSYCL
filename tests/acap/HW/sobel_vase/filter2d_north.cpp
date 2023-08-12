/*
  RUN: %{execute}%s
*/

#include <fstream>
#include <iostream>
#include <sycl/sycl.hpp>

// OpenCV Includes
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace sycl::vendor::xilinx;

// hardcode the width / height / bpp for simplicity
static constexpr auto width = 800;
static constexpr auto height = 600;
static constexpr auto bpp = 2;

static constexpr auto num_pixels_total = width * height;
static constexpr auto num_bytes_total = num_pixels_total * bpp;

// slice a 800x600 into 2 lines. 4 lines to be given as input
static constexpr auto num_output_lines_per_slice = 2;
static constexpr auto num_output_bytes_per_slice =
  width * num_output_lines_per_slice * bpp;
static constexpr auto num_input_lines_per_slice =
  num_output_lines_per_slice + 2;
static constexpr auto num_input_bytes_per_slice =
  width * num_input_lines_per_slice * bpp;;

// static memory and pointer
static uint32_t *buffer;
static uint32_t out_buffer[num_bytes_total / 4];

/**

    North Side only variation of 2d filter based on sobel example, simpler than
    the east only memory implementation, but having both is a nice showcase of
    the runtime difference of the different memory locations

----------------------

    HOST Addresses i.e. how the PS/AXI-MM(?) percieves/maps the tiles address
    memory refer to 2.9.2 ME Array Overall Address Map in Arch Spec -

    Borrowed means the memory is technically owned by a neighbouring tile,
    checker pattern for a visual reference of memory ownership.

    East Address on odd (borrowed): 0x800000, East address on even (owner): 0x0
    North address on odd/even (borrowed never owned): 0x40000

----------------------
    ME Tile Addresses, i.e. how the address space is percieved/mapped from the
    Tiles perspective, refer to Table 5-15 of section 5.5.1.2 in the Arch Spec

    South Address Space: 0x00020000 – 0x00027FFF
    West  Address Space: 0x00028000 – 0x0002FFFF
    North Address Space: 0x00030000 – 0x00037FFF
    Eeast Address Space: 0x00038000 – 0x0003FFFF

    This Address Space mapping is why the address space for the buffer memory we
    allocate passed across to the Tile in post run is different from the hosts
    concept of the address. We're writing our data to the east memory of the
    tile, so all of our data starts offset from:

      0x00038000 (0x38000)

    if we were doing it for some north allocated memory we would be passing our:

    data offset + 0x00030000

    To get the appropriate address

----------------------

    \param AIE is an implementation-defined type
    \param X is the horizontal coordinate
    \param Y is the vertical coordinate
*/
template <typename AIE, int X, int Y>
struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  // Pre-run, checks the valid tile locations, pushes args and data
  int prerun() {
    uint32_t offset, tile_offset;
    uint32_t buffer_offset = ((t::geo::x_size * Y + X) *
        num_output_bytes_per_slice) / 4;

    // each takes 2 lines, so only 299 tiles are needed as the top / bottom
    // lines are skpped.
    if constexpr (t::geo::x_size * Y + X >= 299) {
      return 0;
    }

    // the north memory is not part of the given partition
    // this check is still needed with above one, ex when only partial columns
    // are given to this array.
    if constexpr (Y == t::geo::y_size - 1) {
      return 0;
    }

    // north: 'offset' is from the host core address space view.
    // 'tile_offset' is from the aie core address space view.
    offset = 0x40000;
    tile_offset = 0x30000;

    // TODO: let's clear stack, just to make sure. the stack locaiton (0x300)
    // is, in fact, specified in bcf linker script, generated
    // by sycl-aie compiler.
    for (unsigned int i = 0; i < 0x300 / 4; i++)
      t::mem_write(offset + i * 4, 0);

    // function call args
    // TODO: all these may be handled by tool or runtime module.
    // - argument passing by compiler
    // - memory allocation by runtime module
    // arg0 filter location:  0x4 * 8
    t::mem_write(offset + 0x4 * 0, tile_offset + 0x4 * 8);
    // arg1 input:  0x300
    t::mem_write(offset + 0x4 * 1, tile_offset + 0x300);
    // arg2 output:  0x4e00
    t::mem_write(offset + 0x4 * 2, tile_offset + 0x4e00);
    // arg3 0d start offset
    t::mem_write(offset + 0x4 * 3, 0);
    // arg4 0d width: in 16bit
    t::mem_write(offset + 0x4 * 4, 800 * bpp / 2);
    // arg5 1d start offset
    t::mem_write(offset + 0x4 * 5, 0);
    // arg6 1d width
    t::mem_write(offset + 0x4 * 6, 2);
    // arg7 1d stride: in 16bit
    t::mem_write(offset + 0x4 * 7, 800 * bpp / 2);
    // arg8 1 3x3 filter: stride = 4 to be compatible with vector ops
    for (unsigned int i = 0; i < 12 / 2; i++) {
      int16_t filter[] = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 }; // smooth
      t::mem_write(offset + 0x4 * (8 + i), filter[i * 2 + 1] << 16 | filter[i * 2]);
    }

    // Write some data to memory
    for (unsigned int i = 0; i < num_input_bytes_per_slice / 4; i++) {
      t::mem_write(offset + 0x300 + i * 4, buffer[buffer_offset + i]);
    }

    // clear the output data, just to make sure
    for (unsigned int i = 0; i < num_output_bytes_per_slice / 4; i++)
      t::mem_write(offset + 0x4e00 + i * 4, 0);

    // set perf counter, manually for the first tile
    if constexpr (Y == 0 && X == 0) {
      // count core active (= 28) cycles
      t::mem_write(0x31000, 28 << 8 | 28);
    }

    return 1;
  }

  // Post-run, gets the core cyles from perf counter, and takes the result
  // out of the data memory.
  void postrun() {
    uint32_t offset, tile_offset;
    uint32_t buffer_offset = ((t::geo::x_size * Y + X)
        * num_output_bytes_per_slice) / 4;

    // read the perf counter, manually.
    // The result is identical across tiles anyway
    if constexpr (Y == 0 && X == 0) {
      // read core active cycles
      std::cout << "loc " << X << ", " << Y << " active cycles " <<
        t::mem_read(0x31020) << std::endl;
      // clear counter
      t::mem_write(0x31020, 0);
    }

    // north: 'offset' is from the host core address space view.
    offset = 0x40000;

    // Read the data
    for (unsigned int i = 0; i < num_output_bytes_per_slice / 4; i++) {
      out_buffer[buffer_offset + i] = t::mem_read(offset + 0x4e00 + i * 4);
    }
  }

  int16_t *arg0;
  int16_t *arg1;
  int16_t *arg2;
  int arg3;
  int arg4;
  int arg5;
  int arg6;
  int arg7;

  void run() {
    // TODO: these better be, automatically, aligned with above args in prerun()
    int16_t *_filter_buffer =             (int16_t *)     arg0;
    int16_t *_input_buffer =              (int16_t *)     arg1;
    int16_t *_conv_buffer =               (int16_t *)     arg2;
    int _conv_buffer_dim0_min =           (int)           arg3;
    int _conv_buffer_dim0_extent =        (int)           arg4;
    int _conv_buffer_dim1_min =           (int)           arg5;
    int _conv_buffer_dim1_extent =        (int)           arg6;
    int _conv_buffer_dim1_stride =        (int)           arg7;

    // number of filters can be passed as arg too
    for (uint32_t f = 0; f < 1; f++) {
      for (uint32_t h = _conv_buffer_dim1_min; h < _conv_buffer_dim1_extent; h++) {
        for (uint32_t w = _conv_buffer_dim0_min; w < _conv_buffer_dim0_extent; w++) {
          int16_t result = 0;

          // size / stride of filter as arg too.
          for (uint32_t x = 0; x < 3; x++) {
            for (uint32_t y = 0; y < 3; y++) {
              uint32_t index = (w + x) + (h + y) * _conv_buffer_dim1_stride;
              uint32_t index_filter = x * 4 + y;

              result += _filter_buffer[index_filter] * _input_buffer[index];
            }
          }
          // divide by 8 just to match with vector op (divide with bit shift)
          _conv_buffer[h * _conv_buffer_dim1_stride + w + 1] = result / 8;
        }
      }
    }
  }
};


int main() {
  acap::aie::device<acap::aie::layout::vc1902> aie;

  std::ofstream inputFile;
  std::ofstream outputFile;

  cv::Mat inputColor = cv::imread("vase2.bmp");
  cv::Mat inputRaw, input;
  cv::cvtColor(inputColor, inputRaw, CV_BGR2GRAY);
  inputRaw.convertTo(input, CV_16UC1);

  cv::imwrite("input.bmp", input);

  buffer = (uint32_t *)input.data;

  inputFile.open("lab-800x600-sobel.data");
  inputFile.write((char *)buffer, input.total() * bpp);
  inputFile.close();

  // Not really neccessary but if the resulting image isn't all black we're at
  // least sure something happened on the read back
  for (int i = 0; i < num_bytes_total / 4; ++i)
    out_buffer[i] = 0;

  aie.run<prog>();

  cv::Mat outputRaw(std::vector{height, width}, CV_16UC1, (char *)out_buffer);
  cv::Mat output;

  outputRaw.convertTo(output, CV_8UC1);
  cv::imwrite("output.bmp", output);

  outputFile.open("lab-800x600-sobel-aie.data");
  outputFile.write((char *)out_buffer, width * height * bpp);
  outputFile.close();

  return 0;
}
