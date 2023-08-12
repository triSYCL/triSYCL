/* Simple hello world program for ACAP/AI Engine hardware

   NOTE: This requires tweaking to work with the current compiler, for example
   the tile functions relating to reset/etc are pushed down into the API and the
   kernel doesn't need to be commented out. But also some more lower level
   components like the linker script have changed and the main file (most of the
   earlier iterations were a hybrid of Cardano Tool Flow and SYCL Frontend flow)

   RUN: %{execute}%s
*/

#include <iostream>
#include <cstring>
#include <sycl/sycl.hpp>

using namespace sycl::vendor::xilinx;

constexpr int elements_per_tile = 8;
// becareful with array_x/array_y and the Tiles X and Y, the Tiles X and Y
// starts at 0 and goes to max - 1 like a normal index, but the array_x and
// array_y start at 1 and go to max. This is the only location they're used
// though.
constexpr int array_x = 50;
constexpr int array_y = 7;
constexpr int total_elements = array_x * array_y * elements_per_tile;

// This has to be external to the functor/tile program, you will ICE the
// compiler, this is a bug in the Intel implementation, but it's likely the fix
// to it will be an error diagnostic emitted rather than a nice fix that allows
// it to be useable like we'd want..
//
// This is unfortunate as it would be nice to move the constexpr stuff above into
// the array and make the size based on the tile programs maximum extents which
// makes this a lot easier to read...
float sycl_buff[total_elements];

/** A small AI Engine program

    The definition of a tile program has to start this way

    \param AIE is an implementation-defined type

    \param X is the horizontal coordinate

    \param Y is the vertical coordinate
*/
template <typename AIE, int X, int Y>
struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  // north address space for both, just different views
  uint32_t host_address_offset = 0x40000;
  uint32_t aie_tile_address_offset = 0x30000;
  uint32_t arg_register_offset = 0x800; // Stack size taken up by arg regiser
                                        // at the moment

  bool prerun() {
    // technically no board larger than 8 rows, and if there was it'd be fine to
    // write to 7. The current implementation relies on north memory so
    // unfortunately the top row is off-limits as it has no north memory
    if constexpr (Y >= 7)
      return 0;


    // Setting the location we will be placing our buffer into the arg register
    // that we have reserved space for on the stack
    t::mem_write(host_address_offset, aie_tile_address_offset +
                 arg_register_offset);

    float sycl_buff[8];
    // Write some data to memory
    for (unsigned int i = 0; i < elements_per_tile; i++) {
      std::cout << "Tile X: " << X << "Y: "
                << Y << "Writing: " << i * 2.2f + X + Y << "\n";
      // This is actually undefined behaviour, the version in postrun is not
      // it's legal and has no undefined behaviour, but I left this in to
      // demonstrate both ways to convert a float to a uint32_t for transfer.
      // Realistically though this needs to be hidden from the user..
      sycl_buff[i] = i * 2.2f + X + Y;
      t::mem_write(host_address_offset + arg_register_offset + i * 4,
                   *reinterpret_cast<unsigned long*>(&sycl_buff[i]));
    }

    return 1;
  }

  /// The run member function is defined as the tile program
  float* sycl_buff_arg;
  void run() {
    for (unsigned int i = 0; i < elements_per_tile; i++) {
      sycl_buff_arg[i] += 2.0;
    }
  }

  void postrun() {
    uint32_t buffer_offset = ((acap::aie::layout::vc1902::x_max + 1) * Y + X)
        * elements_per_tile;

    std::uint32_t tmp;
    for (unsigned int i = 0; i < elements_per_tile; i++) {
      tmp = t::mem_read(host_address_offset + arg_register_offset + i * 4);
      std::memcpy(&sycl_buff[buffer_offset + i], &tmp, sizeof(std::uint32_t));
      assert(sycl_buff[buffer_offset + i] !=  (i * 2.2f + X + Y) + 2.0);
    }
  }
};

int main() {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  acap::aie::device<acap::aie::layout::vc1902> aie;
  // Run up to completion of all the tile programs
  aie.run<prog>();

  for (int i = 0; i < total_elements; ++i)
    std::cout << "sycl_buff return val: " << sycl_buff[i] << "\n";

  return 0;
}
