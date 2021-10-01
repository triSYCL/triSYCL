/// Example of using neighbor memory tiles

#include <sycl/sycl.hpp>
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
  void lock() { t::get_self_lock(0).acquire(); }
  bool prerun() {
    written_data.push_back({X, Y, 0, id_counter++});
    /// writes to tile_memory::x
    t::get_dev_handle().mem_write(acap::hw::tile_mem_begin_offset, written_data.back().x);
    /// writes to tile_memory::y
    t::get_dev_handle().mem_write(acap::hw::tile_mem_begin_offset + 4, written_data.back().y);
    /// writes to tile_memory::id
    t::get_dev_handle().mem_write(acap::hw::tile_mem_begin_offset + 8, written_data.back().id);
    t::get_self_lock(0).release();
    return 1;
  }

  enum dir_idx {
    idx_self = 0,
    idx_north = 1,
    idx_south = 2,
    idx_east = 3,
    idx_west = 4,
  };

  pos output[5];
  void run() {
    /// all uninitialized data will be 0xff instead of 0 to make spotting
    /// uninitialized data easier.
    /// If any data has an id of 0 it means a there is a synchronization issue.
    /// This means pointers will be 0xffffffff and ints will be -1.
    __builtin_memset(this, 0xff, sizeof(*this));

    t::get_self_lock(0).acquire();
    output[idx_self] = {t::mem().x, t::mem().y, (int32_t)&t::mem(), t::mem().id};
    t::get_self_lock(0).release();
    if constexpr (t::is_memory_module_north()) {
      t::get_lock(t::dir::north, 0).acquire();
      output[idx_north] = {t::mem_north().x, t::mem_north().y, (int32_t)&t::mem_north(),
                   t::mem_north().id};
      t::get_lock(t::dir::north, 0).release();
    }
    if constexpr (t::is_memory_module_south()) {
      t::get_lock(t::dir::south, 0).acquire();
      output[idx_south] = {t::mem_south().x, t::mem_south().y, (int32_t)&t::mem_south(),
                   t::mem_south().id};
      t::get_lock(t::dir::south, 0).release();
    }
    if constexpr (t::is_memory_module_east()) {
      t::get_lock(t::dir::east, 0).acquire();
      output[idx_east] = {t::mem_east().x, t::mem_east().y, (int32_t)&t::mem_east(),
                   t::mem_east().id};
      t::get_lock(t::dir::east, 0).release();
    }
    if constexpr (t::is_memory_module_west()) {
      t::get_lock(t::dir::west, 0).acquire();
      output[idx_west] = {t::mem_west().x, t::mem_west().y, (int32_t)&t::mem_west(),
                   t::mem_west().id};
      t::get_lock(t::dir::west, 0).release();
    }
  }

  bool validate(pos p) {
    return std::find_if(written_data.begin(), written_data.end(), [&](pos e) {
             return e.id == p.id && e.x == p.x && e.y == p.y;
           }) != written_data.end();
  }

  void postrun() {
    t::get_dev_handle().memcpy_d2h(&output, acap::hw::args_begin_offset, sizeof(output));
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

      ss << std::endl;
      /// The data was read by the tile before it was initialized by the host.
      /// This shouldn't be possible because of locks so its a bug
      assert(output[i].id != 0 && " synchronization bug");
      /// The data that was written doesn't match that data being read back.
      if (output[i].id != -1)
        assert(validate(output[i]) && " invalid data bug");
    }
    ss << std::endl;
    std::cout << ss.str();
    assert(output[idx_self].x == X && output[idx_self].y == Y);
    if (t::is_memory_module_north())
      assert(output[idx_north].x == X && output[idx_north].y == Y + 1 &&
             output[idx_north].id != 0);
    else
      assert(output[idx_north].x == -1 && output[idx_north].y == -1 && output[idx_north].id == -1);
    if (t::is_memory_module_south())
      assert(output[idx_south].x == X && output[idx_south].y == Y - 1 &&
             output[idx_south].id != 0);
    else
      assert(output[idx_south].x == -1 && output[idx_south].y == -1 && output[idx_south].id == -1);
    if (Y & 1) {
      assert(output[idx_west].x == X && output[idx_west].y == Y &&
             output[idx_west].id == output[idx_self].id);
      if (t::is_memory_module_east())
        assert(output[idx_east].x == X + 1 && output[idx_east].y == Y &&
               output[idx_east].id != 0);
      else
        assert(output[idx_east].x == -1 && output[idx_east].y == -1 &&
               output[idx_east].id == -1);
    } else {
      assert(output[idx_east].x == X && output[idx_east].y == Y &&
             output[idx_east].id == output[idx_self].id);
      if (t::is_memory_module_west())
        assert(output[idx_west].x == X - 1 && output[idx_west].y == Y &&
               output[idx_west].id != 0);
      else
        assert(output[idx_west].x == -1 && output[idx_west].y == -1 &&
               output[idx_west].id == -1);
    }
  }
};

int main(int argc, char **argv) {
  // Define AIE CGRA running a program "prog" on all the tiles of a VC1902
  // acap::aie::device<acap::aie::layout::vc1902> aie;
  acap::aie::device<acap::aie::layout::size<5, 5>> aie;
  // Run up to completion of all the tile programs
  aie.run<prog, tile_memory>();

  return 0;
}
