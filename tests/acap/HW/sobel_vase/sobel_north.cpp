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

#define NUM_PIXELS_TOTAL                  (800 * 600)
#define NUM_INPUT_BYTES_TOTAL             (NUM_PIXELS_TOTAL * 1)
#define NUM_OUTPUT_BYTES_TOTAL            (NUM_PIXELS_TOTAL * 1)

#define NUM_PIXELS_PER_SLICE              (800 * 4)
#define NUM_INPUT_BYTES_PER_SLICE         (NUM_PIXELS_PER_SLICE * 1)
#define NUM_INPUT_OFFSET_BYTES_PER_SLICE  (800 * 2 * 1)
#define NUM_OUTPUT_BYTES_PER_SLICE        (800 * 2 * 1)


static uint32_t *buffer;
static uint32_t out_buffer[NUM_OUTPUT_BYTES_TOTAL / 4];

/**

    North Side only variation of Sobel based on Hyun's example, simpler than the
    east only memory implementation, but having both is a nice showcase of the
    runtime difference of the different memory locations

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

  /// Pre-run, the pre-run and post-run in this program only use the tiles east
  /// memory
  int prerun() {
    uint32_t offset, tile_offset;
    uint32_t xmax = acap::aie::layout::vc1902::x_max;
    uint32_t buffer_offset = (((xmax + 1) * Y + X)
        * NUM_INPUT_OFFSET_BYTES_PER_SLICE) / 4;

    /// each takes 2 lines, so only 300 tiles are needed
    /// top 2 rows are not used
    if constexpr (Y > 5) {
      return 0;
    }

    /// north
    offset = 0x40000;
    tile_offset = 0x30000;

    /// function call args
    /// input: 0x800
    t::mem_write(offset, tile_offset + 0x800);
    /// output: 0x4800
    t::mem_write(offset + 0x4 * 1, tile_offset + 0x4800);
    /// width
    t::mem_write(offset + 0x4 * 2, 800);
    /// height
    t::mem_write(offset + 0x4 * 3, 4);

    // Write some data to memory
    for (unsigned int i = 0; i < NUM_INPUT_BYTES_PER_SLICE / 4; i++)
      t::mem_write(offset + 0x800 + i * 4, buffer[buffer_offset + i]);

    return 1;
  }

    /// Post-run
  void postrun() {
    uint32_t offset, tile_offset;
    uint32_t xmax = acap::aie::layout::vc1902::x_max;
    uint32_t buffer_offset = (((xmax + 1) * Y + X)
        * NUM_OUTPUT_BYTES_PER_SLICE) / 4;

    /// north
    offset = 0x40000;
    tile_offset = 0x30000;

    /// Read the data
    for (unsigned int i = 0; i < NUM_OUTPUT_BYTES_PER_SLICE / 4; i++) {
      out_buffer[buffer_offset + i] = t::mem_read(offset + 0x4800 + i * 4);
    }

  }

  uint8_t *arg0;
  uint8_t *arg1;
  uint32_t arg2;
  uint32_t arg3;

  void run() {
    auto input = arg0;
    auto output = arg1;
    auto width = arg2;
    auto height = arg3;

    for (uint32_t w = 1; w < width - 1; ++w) {
      for (uint32_t h = 1; h < height - 1; ++h) {
        int32_t result_x = 0, result_y = 0;
        int8_t hori[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        int8_t vert[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

        for (uint32_t x = 0; x < 3; x++) {
          for (uint32_t y = 0; y < 3; y++) {
            uint32_t index = (w + x - 1) + (h + y - 1) * width;
            uint32_t index_filter = x * 3 + y;

            result_x += hori[index_filter] * input[index];
            result_y += vert[index_filter] * input[index];
          }
        }
        output[w + (h - 1) * width] = (abs(result_x) + abs(result_y) > 0xff ?
                        0xff : (abs(result_x) + abs(result_y)));
      }
    }
  }
};


int main() {
  acap::aie::device<acap::aie::layout::vc1902> aie;

  std::ofstream inputFile;
  std::ofstream outputFile;

  constexpr auto height = 600; // input.rows;
  constexpr auto width = 800; // input.cols;

  cv::Mat inputColor = cv::imread("vase2.bmp");
  cv::Mat inputRaw, input;
  cv::cvtColor(inputColor, inputRaw, CV_BGR2GRAY);
  inputRaw.convertTo(input, CV_8UC1);

  cv::imwrite("input.bmp", input);

  buffer = (uint32_t *)input.data;

  inputFile.open("lab-800x600-sobel.data");
  inputFile.write((char *)buffer, input.total());
  inputFile.close();

  // Not really neccessary but if the resulting image isn't all black we're at
  // least sure something happened on the read back
  for (int i = 0; i < NUM_OUTPUT_BYTES_TOTAL / 4; ++i)
    out_buffer[i] = 0;

  aie.run<prog>();

  cv::Mat outputMat(std::vector{600, 800}, CV_8UC1, (char *)out_buffer);
  cv::imwrite("output.bmp", outputMat);

  outputFile.open("lab-800x600-sobel-aie.data");
  outputFile.write((char *)out_buffer, 800 * 600);
  outputFile.close();

  return 0;
}
