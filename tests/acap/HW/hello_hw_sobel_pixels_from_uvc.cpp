/* Simple hello world program for ACAP/AI Engine hardware
   RUN: %{execute}%s
*/

#include <fstream>
#include <iostream>
#include <SYCL/sycl.hpp>

// OpenCV Includes
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define NUM_PIXELS_TOTAL		(800 * 600)
#define NUM_INPUT_BYTES_TOTAL		(NUM_PIXELS_TOTAL * 2)	//yuyv = 2 bytes per pixel
#define NUM_OUTPUT_BYTES_TOTAL		(NUM_PIXELS_TOTAL * 1)	//gray = 1 byte per pixel

#define NUM_PIXELS_PER_SLICE		(800 * 4)
#define NUM_INPUT_BYTES_PER_SLICE	(NUM_PIXELS_PER_SLICE * 2) //yuyv = 2 bytes per pixel
#define NUM_INPUT_OFFSET_BYTES_PER_SLICE	(800 * 2 * 2) //yuyv = 2 bytes per pixel
#define NUM_OUTPUT_BYTES_PER_SLICE	(800 * 2 * 1) //gray = 1 byte per pixel

using namespace sycl::vendor::xilinx;

static uint32_t *buffer;
static uint32_t out_buffer[NUM_OUTPUT_BYTES_TOTAL / 4];

/** A vc1902 AI Engine program
    The definition of a tile program has to start this way
    \param AIE is an implementation-defined type
    \param X is the horizontal coordinate
    \param Y is the vertical coordinate
*/
template <typename AIE, int X, int Y>
struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  /// Pre-run
  int prerun() {
    uint32_t offset = 0;
    uint32_t xmax = acap::aie::layout::vc1902::x_max;
    uint32_t buffer_offset = (((xmax + 1) * Y + X) * NUM_INPUT_OFFSET_BYTES_PER_SLICE) / 4;

    /// each takes 2 lines, so only 300 tiles are needed
    if constexpr (((acap::aie::layout::vc1902::x_max + 1) * Y + X) > 300) {
      std::cout << "Skipping excessive tiles " << X << ',' << Y  << std::endl;
      return -1;
    }
    /// right column without memory can't access east memory
    if constexpr (t::is_right_column() && (Y & 1)) {
      std::cout << "Skipping " << X << ',' << Y  << std::endl;
      return -1;
    } else {
//      std::cout << "Hello, I am the AIE tile (" << X << ',' << Y  << ")"
//                << std::endl;
    }

    /// FIXME: place it here for now
    t::core_reset();

    /* FIXME: maybe accessor taking memory module as argument is cleaner */
    /// access the east memory
    if constexpr (Y & 1)
      offset += 0x800000;

    /// function call args
    /// input: 0x800
    t::mem_write(offset, 0x38800);
    /// output: 0x4800
    t::mem_write(offset + 0x4 * 1, 0x3c800);
    /// width
    t::mem_write(offset + 0x4 * 2, 800);
    /// height
    t::mem_write(offset + 0x4 * 3, 6);

    // Write some data to memory
    for (unsigned int i = 0; i < NUM_INPUT_BYTES_PER_SLICE / 4; i++)
      t::mem_write(offset + 0x800 + i * 4, buffer[buffer_offset + i]);

    return 0;
  }

  /// Post-run
  void postrun() {
    uint32_t offset = 0;
    uint32_t xmax = acap::aie::layout::vc1902::x_max;
    uint32_t buffer_offset = (((xmax + 1) * Y + X) * NUM_OUTPUT_BYTES_PER_SLICE) / 4;

    /// FIXME: place it here for now
    t::core_run();
    t::core_wait();
    t::core_stop();

    /* FIXME: maybe accessor taking memory module as argument is cleaner */
    /// access the east memory
    if constexpr (Y & 1)
      offset += 0x800000;

    /// Read the data
    for (unsigned int i = 0; i < NUM_OUTPUT_BYTES_PER_SLICE / 4; i++) {
      if (buffer_offset + i > 480000 / 4) {
        std::cout << "error " << X << ',' << Y  << " " << buffer_offset + i << std::endl;
        break;
      }
      out_buffer[buffer_offset + i] = t::mem_read(offset + 0x4800 + i * 4);
    }
  }

  /// The run member function is defined as the tile program
  void run() {
    /// This has to be compiled by the device compiler.
    /// for (uint32_t w = 1; w < width - 1; w++) {
    ///   for (uint32_t h = 1; h < height - 1; h++) {
    ///     int32_t result_x = 0, result_y = 0;
    ///     uint8_t result = 0;
    ///     int8_t hori[] = {-1, 0, 1, -2, 0, 2, -1, 0, -1};
    ///     int8_t vert[] = {1, 2, 1, 0, 0, 0, -1, 0, -1};
    ///
    ///     for (uint32_t x = 0; x < 3; x++) {
    ///       for (uint32_t y = 0; y < 3; y++) {
    ///         uint32_t index = (w + x - 1) + (h + y - 1) * width;
    ///         uint32_t index_filter = x * 3 + y;
    ///
    ///         result_x += hori[index_filter] * ((input[index] & 0xff));
    ///         result_y += vert[index_filter] * ((input[index] & 0xff));
    ///       }
    ///     }
    ///     output[w + h * width] = (abs(result_x) + abs(result_y) > 0xff ?
    ///                     (abs(result_x) + abs(result_y)) : 0xff);
    ///   }
    /// }

    {
    	t::load_elf("aie-sobel.elf");
    }
  }
};

int main() {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  acap::aie::array<acap::aie::layout::vc1902, prog> aie;

  std::ofstream input;
  std::ofstream output;

  constexpr auto height = 600; // input.rows;
  constexpr auto width = 800; // input.cols;

  cv::VideoCapture cap;
  cv::VideoWriter writer;
  cv::Mat captureRAW, captureGRAY, capture;

  // If opencv don't find a webcam the program exit here
  if (!cap.open(0)) {
    std::cerr << "Unable to connect to the webcam" << std::endl;
    return 1;
  }

  // set the size of picture taken by the webcam
  cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
  // don't do software convert
  cap.set(cv::CAP_PROP_CONVERT_RGB, 0);
  // use yuyv format to skip conversion / decompression
  cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));

  cap >> captureRAW;
  cv::cvtColor(captureRAW, captureGRAY, cv::COLOR_YUV2GRAY_YUYV);
  captureGRAY.convertTo(capture, CV_8UC1);

  buffer = (uint32_t *)capture.data;

  input.open("lab-800x600-sobel.data");
  input.write((char *)buffer, capture.total());
  input.close();

  buffer = (uint32_t *)captureRAW.data;

  // Run up to completion of all the tile programs
  aie.run();

  output.open("lab-800x600-sobel-aie.data");
  output.write((char *)out_buffer, 800 * 600);
  output.close();
}

