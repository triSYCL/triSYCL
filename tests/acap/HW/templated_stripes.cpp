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
#define NUM_OUTPUT_BYTES_TOTAL            (NUM_PIXELS_TOTAL * 1)

#define NUM_PIXELS_PER_SLICE              (800 * 4)
#define NUM_OUTPUT_BYTES_PER_SLICE        (800 * 2 * 1)

static uint32_t out_buffer[NUM_OUTPUT_BYTES_TOTAL / 4];

/**
     Simple test to check if we are generating unique tiles correctly by
     creating some tiles that will set pixels to black and others that will set
     them to white based on the tile ID being odd or even. The output pattern
     is stripes in this particular case.

     Uses north memory.

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
    /// each takes 2 lines, so only 300 tiles are needed
    /// top 2 rows are not used
    if constexpr (Y > 5) {
      return 0;
    }

    /// north
    uint32_t offset = 0x40000;
    uint32_t tile_offset = 0x30000;

    /// function call args
    /// output address: 0x800
    t::mem_write(offset, tile_offset + 0x800);
    /// width
    t::mem_write(offset + 0x4 * 1, 800);
    /// height
    t::mem_write(offset + 0x4 * 2, 4);

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
      out_buffer[buffer_offset + i] = t::mem_read(offset + 0x800 + i * 4);
    }
  }

  // Linker Script Gen:
  // See linker_script.bcf TODO, but essentially make each tile use its own
  // memory rather than a neighbours
  uint8_t *output;
  uint32_t width;
  uint32_t height;
  void run() {
    for (int x = 0; x < width; ++x)
      for (int y = 0; y < height; ++y) {
       if constexpr ((X + Y) & 1)
         output[width * y + x] = 255;
       else
         output[width * y + x] = 0;
      }
  }
};


int main() {
  acap::aie::device<acap::aie::layout::vc1902> aie;

  std::ofstream outputFile;

  constexpr auto height = 600; // input.rows;
  constexpr auto width = 800; // input.cols;

  aie.run<prog>();

  cv::Mat outputMat(std::vector{600, 800}, CV_8UC1, (char *)out_buffer);
  cv::imwrite("output.bmp", outputMat);

  outputFile.open("lab-800x600-sobel-aie.data");
  outputFile.write((char *)out_buffer, 800 * 600);
  outputFile.close();

  return 0;
}
