/// Example of using neighbor memory tiles

#include <sycl/sycl.hpp>
#include <cstring>
#include <iostream>
#include <vector>

using namespace sycl::vendor::xilinx;

struct pos {
  int x;
  int y;
  int ptr;
  volatile int id;
};

int id_counter = 17;

/// A memory tile has to inherit from acap::aie::memory<AIE, X, Y>
template <typename AIE, int X, int Y>
struct tile_memory : acap::aie::memory<AIE, X, Y> {
  int x;
  int y;
  int id;
};

template <typename AIE, int X, int Y> struct prog : acap::aie::tile<AIE, X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  bool prerun() {
    /// writes to tile_memory::x
    t::mem_write(acap::hw::tile_mem_begin_offset, X);
    /// writes to tile_memory::y
    t::mem_write(acap::hw::tile_mem_begin_offset + 4, Y);
    /// writes to tile_memory::id
    t::mem_write(acap::hw::tile_mem_begin_offset + 8, id_counter++);
    return 1;
  }

  pos output[5];
  void run() {
    /// all uninitialized data will be 0xff instead of 0 to make spotting
    /// uninitialized data easier.
    /// This means pointers will be 0xffffffff and ints will be -1.
    __builtin_memset(this, 0xff, sizeof(*this));

    output[0] = {t::mem().x, t::mem().y, (int32_t)&t::mem(), t::mem().id};
    if constexpr (t::is_memory_module_north()) {
      /// spinlock on the tile being initialized by the host before accessing
      /// it. This only works because t::mem_* function return a reference on
      /// volatile(on device), so spinlock loops like this one are not dead.
      while (t::mem_north().id == 0)
        ;
      output[1] = {t::mem_north().x, t::mem_north().y, (int32_t)&t::mem_north(),
                   t::mem_north().id};
    }
    if constexpr (t::is_memory_module_south()) {
      while (t::mem_south().id == 0)
        ;
      output[2] = {t::mem_south().x, t::mem_south().y, (int32_t)&t::mem_south(),
                   t::mem_south().id};
    }
    if constexpr (t::is_memory_module_east()) {
      while (t::mem_east().id == 0)
        ;
      output[3] = {t::mem_east().x, t::mem_east().y, (int32_t)&t::mem_east(),
                   t::mem_east().id};
    }
    if constexpr (t::is_memory_module_west()) {
      while (t::mem_west().id == 0)
        ;
      output[4] = {t::mem_west().x, t::mem_west().y, (int32_t)&t::mem_west(),
                   t::mem_west().id};
    }
  }

  void postrun() {
    /// memcpy the device representation of *this into the host one. this is
    /// possible because *this doesn't contain any pointer type so the layout on
    /// host and device is identical.
    t::memcpy_d2h(&output, acap::hw::args_begin_offset, sizeof(output));

    static const char *strs[] = {"self", "up/north", "down/south", "right/east",
                                 "left/west"};

    /// use std::stringstream instead of std::cout to prevent interleaving in
    /// the stdout.
    std::stringstream ss;
    ss << "(" << X << ", " << Y
       << "):" << ((Y & 1) ? "left/west" : "right/east") << std::endl;
    for (int i = 0; i < 5; i++) {

      ss << strs[i] << " (" << output[i].x << ", " << output[i].y << ") = 0x"
         << std::hex << output[i].ptr << std::dec << " id: " << output[i].id
         << std::endl;
    }
    ss << std::endl;
    std::cout << ss.str().c_str();
  }
};

int main(int argc, char **argv) {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  // acap::aie::device<acap::aie::layout::vc1902> aie;
  acap::aie::device<acap::aie::layout::size<3, 3>> aie;
  // Run up to completion of all the tile programs
  aie.run<prog, tile_memory>();

  return 0;
}
