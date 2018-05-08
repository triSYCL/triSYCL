/* Experimenting with processor arrays such as ACAP/MathEngine

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
   Based on Math Engine (ME) Architecture Specification,Revision v1.4
   March 2018
*/

#include <iostream>
#include <type_traits>

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


/** The MathEngine array structure
 */
template <typename ME_Array, typename ME_Memory, typename ME_Program>
struct me_array
  : me_layout {
  ME_Program p;

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


  /** The local data memory to a ME tile

     \todo It looks like there is no data memory on a shim tile
   */
  template <int x, int y, typename Local_Mem>
  struct local_mem {
  };


  /** The MathEngine tile infrastructure
   */
  template <int x, int y, typename Tile>
  struct tile {
    static bool constexpr is_noc_tile() {
      return me_array::is_noc_tile(x, y);
    }

    static bool constexpr is_pl_tile() {
      return me_array::is_pl_tile(x, y);
    }

    static bool constexpr is_shim_tile() {
      return me_array::is_shim_tile(x, y);
    }
  };

};

template <typename Program>
struct program {
  typename Program::template tile<0,0> p;
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

template <typename ME_Array>
struct program
  : acap::program<program<ME_Array>> {
  // Local program of PE(x,y)
  template <int x, int y>
  struct tile {
#if 0
  struct tile : ME_Array::template tile<x, y, tile<x, y>> {
    using base = typename ME_Array::template tile<x, y, tile<x, y>>;

    void hello() {
      std::cout << "Hello, I am the PE " << x << ',' << y << std::endl;
      if constexpr (base::is_shim_tile()) {
        std::cout << "  and I am a shim PE ";
        if constexpr (base::is_noc_tile()) {
            std::cout << "(a NoC controller)" << std::endl;
        }
        if constexpr (base::is_pl_tile()) {
            std::cout << "(a PL interface)" << std::endl;
        }
      }
    }
#endif
  };
};

struct my_me : acap::me_array<my_me,
                              memory<my_me>,
                              program<my_me>> {
};


int main() {

  my_me me;

//  me.run();

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
