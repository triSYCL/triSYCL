/// Example of using neighbor memory tiles

#include <SYCL/sycl.hpp>
#include <cstring>
#include <iostream>
#include <vector>

using namespace sycl::vendor::xilinx;

struct pos {
  int32_t x;
  int32_t y;
  int32_t ptr1;
  int32_t id;
};

std::vector<pos> written_data;
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
  /// The lock function of every tiles is run before the prerun of any other
  /// tile.
  void lock() { t::get_lock(0).acquire(); }
  bool prerun() {
    written_data.push_back({X, Y, 0, id_counter++});
    /// writes to tile_memory::x
    t::mem_write(acap::hw_mem::tile_mem_beg_off, written_data.back().x);
    /// writes to tile_memory::y
    t::mem_write(acap::hw_mem::tile_mem_beg_off + 4, written_data.back().y);
    /// writes to tile_memory::id
    t::mem_write(acap::hw_mem::tile_mem_beg_off + 8, written_data.back().id);
    t::get_lock(0).release();
    return 1;
  }

  pos output[5];
  void run() {
    /// all uninitialized data will be 0xff instead of 0 to make spotting
    /// uninitialized data easier.
    /// If any data has an id of 0 it means a there is a synchronization issue.
    /// This means pointers will be 0xffffffff and ints will be -1.
    __builtin_memset(this, 0xff, sizeof(*this));

    t::get_lock(0).acquire();
    output[0] = {t::mem().x, t::mem().y, (int32_t)&t::mem(), t::mem().id};
    t::get_lock(0).release();
    if constexpr (t::is_memory_module_up()) {
      t::get_lock(0, t::dir::up).acquire();
      output[1] = {t::mem_up().x, t::mem_up().y, (int32_t)&t::mem_up(),
                   t::mem_up().id};
      t::get_lock(0, t::dir::up).release();
    }
    if constexpr (t::is_memory_module_down()) {
      t::get_lock(0, t::dir::down).acquire();
      output[2] = {t::mem_down().x, t::mem_down().y, (int32_t)&t::mem_down(),
                   t::mem_down().id};
      t::get_lock(0, t::dir::down).release();
    }
    if constexpr (t::is_memory_module_right()) {
      t::get_lock(0, t::dir::right).acquire();
      output[3] = {t::mem_right().x, t::mem_right().y, (int32_t)&t::mem_right(),
                   t::mem_right().id};
      t::get_lock(0, t::dir::right).release();
    }
    if constexpr (t::is_memory_module_left()) {
      t::get_lock(0, t::dir::left).acquire();
      output[4] = {t::mem_left().x, t::mem_left().y, (int32_t)&t::mem_left(),
                   t::mem_left().id};
      t::get_lock(0, t::dir::left).release();
    }
  }

  bool validate(pos p) {
    return std::find_if(written_data.begin(), written_data.end(), [&](pos e) {
             return e.id == p.id && e.x == p.x && e.y == p.y;
           }) != written_data.end();
  }

  void postrun() {
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
         << std::hex << output[i].ptr1 << std::dec << " id: " << output[i].id;

      /// Some tiles do not exist like the tile. this is not necessarily a bug
      /// as some tile are just out of bounds.
      if (output[i].id == -1)
        ss << " unavailable tile";
      /// The data was read by the tile before it was initialized by the host.
      /// This shouldn't be possible because of locks so its a bug
      else if (output[i].id == 0)
        ss << " synchronization bug";
      /// The data that was written doesn't match that data being read back.
      else if (!validate(output[i]))
        ss << " invalid data bug";

      ss << std::endl;
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
