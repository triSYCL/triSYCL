/// Example of using neighbor memory tiles

#include <SYCL/sycl.hpp>
#include <cstring>
#include <iostream>
#include <vector>

using namespace sycl::vendor::xilinx;

struct pos {
  int32_t x;
  int32_t y;
  int32_t ptr;
  int32_t id;
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
    t::mem_write(acap::hw_mem::tile_mem_beg_off, X);
    /// writes to tile_memory::y
    t::mem_write(acap::hw_mem::tile_mem_beg_off + 4, Y);
    /// writes to tile_memory::id
    t::mem_write(acap::hw_mem::tile_mem_beg_off + 8, id_counter++);
    return 1;
  }

  pos output[5];
  void run() {
    /// all uninitialized data will be 0xff instead of 0 to make spotting
    /// uninitialized data easier.
    /// This means pointers will be 0xffffffff and ints will be -1.
    __builtin_memset(this, 0xff, sizeof(*this));

    output[0] = {t::mem().x, t::mem().y, (int32_t)&t::mem(), t::mem().id};
    if constexpr (t::is_memory_module_up()) {
      /// spinlock on the tile being initialized by the host before accessing
      /// it. This only works because t::mem_* function return a reference on
      /// volatile(on device), so spinlock loops like this one are not dead.
      while (t::mem_up().id == 0)
        ;
      output[1] = {t::mem_up().x, t::mem_up().y, (int32_t)&t::mem_up(),
                   t::mem_up().id};
    }
    if constexpr (t::is_memory_module_down()) {
      while (t::mem_down().id == 0)
        ;
      output[2] = {t::mem_down().x, t::mem_down().y, (int32_t)&t::mem_down(),
                   t::mem_down().id};
    }
    if constexpr (t::is_memory_module_right()) {
      while (t::mem_right().id == 0)
        ;
      output[3] = {t::mem_right().x, t::mem_right().y, (int32_t)&t::mem_right(),
                   t::mem_right().id};
    }
    if constexpr (t::is_memory_module_left()) {
      while (t::mem_left().id == 0)
        ;
      output[4] = {t::mem_left().x, t::mem_left().y, (int32_t)&t::mem_left(),
                   t::mem_left().id};
    }
  }

  void postrun() {
    // unsigned r = t::mem_read(acap::hw_mem::args_beg_off);
    t::memcpy_d2h(&output, acap::hw_mem::args_beg_off, sizeof(output));
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
    printf(ss.str().c_str());
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
