/* Simple hello world program for ACAP/AI Engine hardware

   NOTE: This requires tweaking to work with the current compiler, for example
   the tile functions relating to reset/etc are pushed down into the API and the
   kernel doesn't need to be commented out. But also some more lower level
   components like the linker script have changed and the main file (most of the
   earlier iterations were a hybrid of Cardano Tool Flow and SYCL Frontend flow)

   RUN: %{execute}%s
*/

#include <fstream>
#include <iostream>
#include <sycl.hpp>

// OpenCV Includes
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

/// 800x600 with 3 bytes per pixel with 54 bytes of header
#define SIZE_BYTES		(800 * 600 * 3)

#define NUM_BYTES_PER_SLICE	(800 * 600 * 3 / 400)

using namespace sycl::vendor::xilinx;

static uint32_t *buffer;

/** A small AI Engine program

    The definition of a tile program has to start this way

    \param AIE is an implementation-defined type

    \param X is the horizontal coordinate

    \param Y is the vertical coordinate
*/
template <typename AIE, int X, int Y>
struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  /// The run member function is defined as the tile program
  void run() {
    /// FIXME: hardcode the offset where the core writes
    uint32_t offset = 0x420;
    uint32_t xmax = acap::aie::layout::vc1902::x_max;
    uint32_t buffer_offset = (((xmax + 1) * Y + X) * NUM_BYTES_PER_SLICE) / 4;

    /// right column without memory can't access east memory
    if constexpr (t::is_right_column() && (Y & 1)) {
      std::cout << "Skipping " << X << ',' << Y  << std::endl;
      return;
    } else {
      std::cout << "Hello, I am the AIE tile (" << X << ',' << Y  << ")" << buffer_offset
                << std::endl;
    }

    /* FIXME: maybe accessor taking memory module as argument is cleaner */
    /// access the east memory
    if constexpr (Y & 1)
      offset += 0x800000;

    // Write some data to memory
    for (unsigned int i = 0; i < NUM_BYTES_PER_SLICE / 4; i++) {
      t::mem_write(offset + i * 4, buffer[buffer_offset + i]);
#if 1
#else
      std::cout << t::mem_read(offset + i * 0x4) << " ";
#endif
    }

    /// This has to be compiled by the device compiler. Rest better be out of run()?
    /*
     * static int sycl_out_buf[8];
     * for (unsigned int i = 0; i < NUM_OF_ELEMENTS; i++) {
     *   sycl_out_buf[i]++;
     *   printf("sycn value %d at sycl %p\n", sycl_out_buf[i], &sycl_out_buf[i]);
     * }
     */
    {
    	t::load_elf("aie-invert.elf");
    }

    t::core_reset();
    t::core_run();
    t::core_wait();
    t::core_stop();

    /// Read the data
    for (unsigned int i = 0; i < NUM_BYTES_PER_SLICE / 4; i++) {
#if 1
      buffer[buffer_offset + i] = t::mem_read(offset + i * 4);
#else
      std::cout << "=" << t::mem_read(offset + i * 0x4) << " ";
#endif
    }

    // overwrite with identifiers
//    t::mem_write(0, X);
//    t::mem_write(0x4, Y);
  }
};

int main() {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> aie;

  std::ofstream input;
  std::ofstream output;

  constexpr auto height = 600; // input.rows;
  constexpr auto width = 800; // input.cols;

  cv::VideoCapture cap;
  cv::Mat capture, captureRAW;

  // If opencv don't find a webcam the program exit here
  if (!cap.open(0)) {
    std::cerr << "Unable to connect to the webcam" << std::endl;
    return 1;
  }

  // set the size of picture taken by the webcam
  cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

  cap >> captureRAW;
  cv::cvtColor(captureRAW, capture, CV_BGR2RGB);

  buffer = (uint32_t *)capture.data;

  input.open("lab-800x600.data");
  input.write((char *)buffer, SIZE_BYTES);
  input.close();

  // Run up to completion of all the tile programs
  aie.run<prog>();

  output.open("lab-800x600-aie.data");
  output.write((char *)buffer, SIZE_BYTES);
  output.close();
}

