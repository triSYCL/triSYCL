/*

  Working on cross compile aiengine hello_world...

  Compile for native with libxaiengine:
    $ISYCL_BIN_DIR/clang++ -std=c++2a  \
      -I/storage/ogozillo/ACAP++/acap-device/include \
      -I/storage/ogozillo/libxaiengine/include \
      -L/storage/ogozillo/libxaiengine/lib \
      `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
      -lxaiengine sobel.cpp

  Two-step native device side compilation (only compiles to LLVM IR and outputs
  the integration header, no host or AI Engine binary, this is primarily to
  test the integration header output):
    $ISYCL_BIN_DIR/clang++ -std=c++2a --sycl \
      -fsycl-use-bitcode \
      -fsycl-header-only-library \
       -Xclang -fsycl-int-header=generated_integration_header.h \
      -I/storage/ogozillo/ACAP++/acap-device/include \
      -I/storage/ogozillo/libxaiengine/include \
      `pkg-config gtkmm-3.0 --cflags` -c sobel.cpp -o kernel.out

  Cross compile with libxaiengine no kernel:
    $ISYCL_BIN_DIR/clang++ -std=c++2a -target aarch64-linux-gnu \
      -mcpu=cortex-a72 \
      --sysroot /net/xsjsycl41/srv/Ubuntu-19.04/arm64-root-server-rw-tmp \
      -I/storage/ogozillo/ACAP++/acap-device/include \
      -I/storage/ogozillo/libxaiengine-aarch64/include \
      -L/storage/ogozillo/libxaiengine-aarch64/lib \
      `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
      -lxaiengine sobel.cpp

  libxaiengine lib and includes reside inside the arm image, but the LD linker
  cross compiling it seems to refuse to find it, even when directed at the correct
  folder. Ldconfig also seems happy with it..

  Cross compile with libxaiengine and kernel:
    $ISYCL_BIN_DIR/clang++ -std=c++2a  -Xclang -fforce-enable-int128  \
      -Xclang -aux-triple -Xclang aarch64-linux-gnu \
      -target aarch64-linux-gnu -mcpu=cortex-a72 -fsycl \
      -fsycl-targets=aie32-xilinx-unknown-sycldevice \
      -fsycl-header-only-library \
      --sysroot /net/xsjsycl41/srv/Ubuntu-19.04/arm64-root-server-rw-tmp \
      -I/storage/ogozillo/ACAP++/acap-device/include \
      -I/storage/ogozillo/libxaiengine-aarch64/include \
      -L/storage/ogozillo/libxaiengine-aarch64/lib \
      `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
      -lxaiengine sobel.cpp

  One step cross-compile with unamed lambda and output generated header for
  inspection (This is the required version for AI Engine):
    $ISYCL_BIN_DIR/clang++ -std=c++2a  -Xclang -fforce-enable-int128  \
      -Xclang -aux-triple -Xclang aarch64-linux-gnu \
      -Xclang -fsycl-int-header=generated_integration_header.h \
      -target aarch64-linux-gnu -mcpu=cortex-a72 -fsycl \
      -fsycl-targets=aie32-xilinx-unknown-sycldevice \
      -fsycl-unnamed-lambda \
      -fsycl-header-only-library \
      --sysroot /net/xsjsycl41/srv/Ubuntu-19.04/arm64-root-server-rw-tmp \
      -I/storage/ogozillo/ACAP++/acap-device/include \
      -I/storage/ogozillo/libxaiengine-aarch64/include \
      -L/storage/ogozillo/libxaiengine-aarch64/lib \
      `pkg-config gtkmm-3.0 --cflags` `pkg-config gtkmm-3.0 --libs` \
      -lxaiengine sobel.cpp

  if you're running qemu-aarch64 to emulate on x86 then you'll probably need to
  add the following to your LD_LIBRARY PATH, otherwise it won't find the so's
  for libmetal, libfsys, libopenamp etc.:
  /net/xsjsycl41/srv/Ubuntu-19.04/arm64-root-server-rw-tmp/usr/local/lib
  /net/xsjsycl41/srv/Ubuntu-19.04/arm64-root-server-rw-tmp/lib/aarch64-linux-gnu



  Cardano.h resides here:

  /proj/xbuilds/HEAD_daily_latest/installs/lin64/Scout/HEAD/cardano/include/

  RUN: %{execute}%s
*/

#include <iostream>
#include <SYCL/sycl.hpp>

using namespace sycl::vendor::xilinx;

/** A small AI Engine program
    The definition of a tile program has to start this way
    \param AIE is an implementation-defined type
    \param X is the horizontal coordinate
    \param Y is the vertical coordinate
*/
template <typename AIE, int X, int Y>
struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;

  uint16_t *arg0;
  uint8_t *arg1;
  uint32_t arg2;
  uint32_t arg3;

  // Immediate TODO:
  // 2: Add Tile main wrapping/linking/compilation..
  //   Possibilities:
  //    1) Genrate it at an Integration Header level and then compile it
  //        We will have more information to work with at this level
  //    2) Generate it at an LLVM level by recreating some mimicry of it
  //        this may be very complex if it requires a lot of Cardano intrinsics
  //    3) Somehow get Cardano to generate it for us/Or look into how Cardano
  //        generates it. Not a huge fan of this as it gives us Cardano as a
  //        dependency
  //
  //    1) Investigate into the changes to main when you add more arguements to
  //       a function
  //    2) How cardano generates its main and if there is a way to do it through
  //       llvm IR? I find this unlikely
  //    3) How to link two objects using xchesscc
  //
  // 2.5: Ask Hyun to test the binary in his example, probably have to manually
  //      name the kernel in some way..? Unless the Main file we generate
  //      handles it then hopefully it's possible to just run it
  // 3: Fix host side to do the right data transfers, check Hyuns example
  // 4: Test it works on AI Engine with a seperate tile program and host program
  // 5: Do a program_manager for the tiles and check we can access appropriate
  //    and required integrated header information
  // 6: Start working out more complex program_manager, compilation process
  //    and tile loader..
  // 7: Add more ACAP++ features
  void run() {
    auto input = arg0;
    auto output = arg1;
    auto width = arg2;
    auto height = arg3;

    for (uint32_t w = 1; w < width - 1; w++) {
      for (uint32_t h = 1; h < height - 1; h++) {
        int32_t result_x = 0, result_y = 0;
        uint8_t result = 0;
        int8_t hori[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        int8_t vert[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

        for (uint32_t x = 0; x < 3; x++) {
          for (uint32_t y = 0; y < 3; y++) {
            uint32_t index = (w + x - 1) + (h + y - 1) * width;
            uint32_t index_filter = x * 3 + y;

            result_x += hori[index_filter] * ((input[index] & 0xff));
            result_y += vert[index_filter] * ((input[index] & 0xff));
          }
        }
        output[w + (h - 1) * width] = (abs(result_x) + abs(result_y) > 0xff ?
                        0xff : (abs(result_x) + abs(result_y)));
      }
    }
  }
};


int main() {
//  acap::aie::array<acap::aie::layout::size<3,4>, prog> aie;

  acap::aie::array<acap::aie::layout::one_pe, prog> aie;

  // I think i can just do a loop with the offset this way..?
  // Because the memory offset may just to relevant to the tile as each tile
  // holds its own AI engine object.
  // The dual loops would have to be constexpr because of the tile accessor
  // which should be fine.

  // Write some data to memory
  uint32_t offset = 0x1c00;
  for (unsigned int i = 0; i < 8; i++) {
    aie.tile<0,0>().mem_write(offset + i * 0x4, i * 2);
#if DEBUG
    std::cout << aie.tile<0,0>().mem_read(offset + i * 0x4) << " ";
#endif
  }

  // Run up to completion of all the tile programs
  aie.run();


  /// Read the data
  for (unsigned int i = 0; i < 8; i++) {
#if DEBUG
    if (aie.tile<0,0>().mem_read(offset + i * 0x4) != (i * 2 + 1)) {
      std::cout << "Fail at " << i << " with " <<
          aie.tile<0,0>().mem_read(offset + i * 0x4) << std::endl;
      break;
    }
#else
    std::cout << "=" << aie.tile<0,0>().mem_read(offset + i * 0x4) << " ";
#endif
  }

  return 0;
}
