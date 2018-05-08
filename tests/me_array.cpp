/* Experimenting with processor arrays such as ACAP/MathEngine

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
   Based on Math Engine (ME) Architecture Specification,Revision v1.4
   March 2018
*/

#include <iostream>
#include <type_traits>
#include <boost/hana.hpp>

namespace cl::sycl::vendor::xilinx::acap {

/** Describe the layout of the MathEngine array
 */
struct me_layout {
  /// Some constrains from Figure 2-8 "ME Array Address Map Example", p. 44
  static auto constexpr x_min = 0;
  static auto constexpr x_max = 5;
  static auto constexpr y_min = 0;
  static auto constexpr y_max = 4;

  static bool constexpr is_noc_tile(int x, int y) {
    return y == 0 && 2 <= x && x <= 3;
  }

  static bool constexpr is_pl_tile(int x, int y) {
    return y == 0 && 2 <= x && x <= 3;
  }
};


/** Some geographic information about the array
 */
struct geography
  : me_layout {
  static bool constexpr is_x_valid(int x) {
    return x_min <= x && x <= x_max;
  }

  static bool constexpr is_y_valid(int y) {
    return y_min <= y && y <= y_max;
  }

  static bool constexpr is_shim_tile(int x, int y) {
    // It could be more optimized, but like that it is clearer
    return is_noc_tile(x, y) || is_pl_tile(x, y);
  }
};

/** The MathEngine array structure
 */
template <template <typename ME_Array, int x, int y> typename Tile>
struct me_array {
  Tile<me_array, 0, 0> t0;
  Tile<me_array, 1, 0> t1;

  void run() {
    t0.run(*this);
    t0.run(*this);
  }

  auto& get_t0() {
    return t0;
  }

  auto& get_t1() {
    return t1;
  }
};


/** The MathEngine tile infrastructure
 */
template <int x, int y>
struct tile
  : geography {

  static bool constexpr is_noc_tile() {
    return geography::is_noc_tile(x, y);
  }

  static bool constexpr is_pl_tile() {
    return geography::is_pl_tile(x, y);
  }

  static bool constexpr is_shim_tile() {
    return geography::is_shim_tile(x, y);
  }
};

}

using namespace cl::sycl::vendor::xilinx;


bool constexpr is_red(int x, int y) {
  return (x + y);
};

struct red {
  float v;
};

struct black {
  int i;
};

template <typename ME_Array>
struct memory {
  // Local memory(x,y) shared by 4 neighbours
  template <int x, int y>
  struct local_mem
    : ME_Array::template local_mem<ME_Array, x, y, local_mem<x, y>>
    , std::conditional_t<is_red(x, y), red, black> {
    int use_count = 0;
  };
};


  template <typename ME_Array, int x, int y>
  struct tile
    : acap::tile<x, y> {
    using acap::tile<x, y>::is_shim_tile;
    int v = 42;

    void run(ME_Array &a) {
      std::cout << "Hello, I am the PE " << x << ',' << y
                << " using " << sizeof(*this) << " bytes of memory "
                << std::endl;
      if constexpr (is_shim_tile()) {
        std::cout << "  and I am a shim PE ";
        if constexpr (acap::tile<x, y>::is_noc_tile()) {
            std::cout << "(a NoC controller)" << std::endl;
        }
        if constexpr (acap::tile<x, y>::is_pl_tile()) {
            std::cout << "(a PL interface)" << std::endl;
        }
      }
      std::cout << "Local v = " << v << std::endl;
      // auto neighbour_v = get_tile<get_id<0>() ^ 1, get_id<1>()>().v;
      std::cout << "Neighbour v = " << a.t1.v << std::endl;
    }
  };

struct my_me
  : acap::me_array<tile> {


};

int main() {

  my_me me;

  me.run();

#if 0

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
