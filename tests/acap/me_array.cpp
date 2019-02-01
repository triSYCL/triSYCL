/* Experimenting with processor arrays such as ACAP/AI Engine

   Some ideas from 2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf

   RUN: %{execute}%s
*/

#include <CL/sycl.hpp>

#include <iostream>

using namespace cl::sycl::vendor::xilinx;

/// A small AI Engine program

bool constexpr is_red(int x, int y) {
  return (x + y) & 1;
};

struct red {
  double d = 5.2;
};

struct black {
  int i = 7;
};

template <int X, int Y>
struct memory
  : std::conditional_t<is_red(X, Y), red, black> {
    int use_count = 0;
};

template <typename AIE, int X, int Y>
struct tile
  : acap::aie::tile<AIE, X, Y>
  , memory<X, Y> {
  using t = acap::aie::tile<AIE, X, Y>;
  int v = 42;

  void run() {
    std::cout << "Hello, I am the ME tile (" << X << ',' << Y
              << ") using " << sizeof(*this) << " bytes of memory "
              << std::endl;
    std::cout << "Local v = " << v << std::endl;
    if constexpr (is_red(X, Y))
      std::cout << " d = " << red::d << std::endl;
    else
      std::cout << " i = " << black::i << std::endl;

    // auto neighbour_v = get_tile<get_id<0>() ^ 1, get_id<1>()>().v;
    //std::cout << "Neighbour v = " << a.t1.v << std::endl;
   // auto neighbour_v = a.template get_tile<2,3>();
  }
};


int main() {
  std::cout << std::endl << "Instantiate big AI Engine:"
            << std::endl << std::endl;
  acap::aie::array<acap::aie::layout::full, tile, acap::aie::memory> aie;
  aie.run();

  std::cout << std::endl << "Instantiate tiny AI Engine:"
            << std::endl << std::endl;
  acap::aie::array<acap::aie::layout::one_pe, tile, acap::aie::memory>
    solitaire_aie;
  solitaire_aie.run();



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
