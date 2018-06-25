/* Testing the MathEngine Memory Module

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
   Based on Math Engine (ME) Architecture Specification, Revision v1.4
   March 2018

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>

using namespace cl::sycl::vendor::xilinx;

/// To have a chessboard-like pattern

bool constexpr is_white(int x, int y) {
  // Bottom left is black
  return (x + y) & 1;
};

struct white {
  double d = 5.2;
};

struct black {
  int i = 7;
};

template <typename Geography, typename ME_Array, int X, int Y>
struct memory
  : acap::me::memory<Geography, ME_Array, X, Y>
  , std::conditional_t<is_white(X, Y), white, black> {
  int use_count = 0;
  int v = 42;

  static auto constexpr is_white() {
    return ::is_white(X, Y);
  };
};

template <typename Geography,
          typename ME_Array,
          typename SuperTile,
          int X,
          int Y>
struct tile : acap::me::tile<Geography,
                             ME_Array,
                             SuperTile,
                             X,
                             Y> {
  using t = acap::me::tile<Geography, ME_Array, SuperTile, X, Y>;

  void run(ME_Array &a) {
   auto &own = t::mem();

   std::cout << "Hello, I am the ME tile (" << X << ',' << Y
              << ") using " << sizeof(*this) << " bytes of memory "
              << std::endl;
    std::cout << "Local v = " << own.v << std::endl;
    if constexpr (t::memory_t::is_white())
      std::cout << " d = " << own.d << std::endl;
    else
      std::cout << " i = " << own.i << std::endl;

    // auto neighbour_v = get_tile<get_id<0>() ^ 1, get_id<1>()>().v;
    //std::cout << "Neighbour v = " << a.t1.v << std::endl;
   // auto neighbour_v = a.template get_tile<2,3>();
  }
};



template <int X>
struct m {
  int data;
};


struct me {
  m<0> mem_0;
  m<1> mem_1;
  m<2> mem_2;
};

template <int X>
struct t {
  m<X> mem_0;
  m<X+1> mem_1;

  template <int XX>
  auto &get_mem() {
    if constexpr (XX == 0)
      return mem_0;
    else
      return mem_1;
  }
};


int main() {
/*
  std::cout << std::endl << "Instantiate big MathEngine:"
            << std::endl << std::endl;
  acap::me::array<acap::me::layout::full,
                  tile,
                  memory> me;
  me.run();
*/
  std::cout << std::endl << "Instantiate tiny MathEngine:"
            << std::endl << std::endl;
  acap::me::array<acap::me::layout::one_pe, tile, memory> solitaire_me;
  solitaire_me.run();
/*
  t tile;
  std::cout << "0: " << tile.get_mem<0>().data << std::endl;
  std::cout << "1: " << tile.get_mem<1>().data << std::endl;
*/

#if 0
  auto static t = boost::hana::make_tuple(1,2,3);
  auto static a=  boost::hana::find_if(
        t
      , [] (auto tile) {
          return true;// tile.x == x && tile.y == y;
        }
                                );


    void compute() {
      if constexpr (!(is_shim() || is_noc())) {
        for(;;) {
          if constexpr (is_red(x,y)) {
            std::lock_guard l { local_mem<x + 1>::lock(1) };
            v += 1.5;
          }
          else {
            std::lock_guard l { local_mem<x + 1, y>::lock(2) };
            i++;
          }
        }
      }
    }
  }
#endif
}
