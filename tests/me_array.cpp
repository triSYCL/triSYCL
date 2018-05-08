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
//  static auto constexpr x_max = 5;
  static auto constexpr x_max = 2;
  static auto constexpr y_min = 0;
  //static auto constexpr y_max = 4;
  static auto constexpr y_max = 1;

  static bool constexpr is_noc_tile(int x, int y) {
    return y == 0 && 2 <= x && x <= 3;
  }

  static bool constexpr is_pl_tile(int x, int y) {
    return y == 0 && ((0 <= x && x <= 1) || (4 <= x && x <= 5));
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
template <template <typename ME_Array, int X, int Y> typename Tile>
struct me_array {

  static auto constexpr tile_coordinates = boost::hana::cartesian_product(
    boost::hana::make_tuple(
        boost::hana::range_c<int, me_layout::x_min, me_layout::x_max + 1>
      , boost::hana::range_c<int, me_layout::y_min, me_layout::y_max + 1>
                            )
                                                                          );

  static auto constexpr generate_tiles = [&] {
    return boost::hana::transform(
        tile_coordinates
      , [](auto coord) {
          return Tile<me_array,
                      boost::hana::at_c<0>(coord),
                      boost::hana::at_c<1>(coord)> {};
        }
                                  );
  };

  decltype(generate_tiles()) tiles = generate_tiles();
  Tile<me_array, 0, 0> t0;
  Tile<me_array, 2, 0> t1;
  Tile<me_array, 5, 4> t2;

  void run() {
    t0.run(*this);
    t1.run(*this);
    t2.run(*this);

    std::cout << "Total size of the tiles: " << sizeof(tiles)
              << " bytes." << std::endl;
  }

  auto& get_t0() {
    return t0;
  }

  auto& get_t1() {
    return t1;
  }

  auto& get_t2() {
    return t2;
  }
};


/** The MathEngine tile infrastructure
 */
template <int X, int Y>
struct tile
  : geography {
  // The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  static bool constexpr is_noc() {
    return geography::is_noc_tile(x, y);
  }

  static bool constexpr is_pl() {
    return geography::is_pl_tile(x, y);
  }

  static bool constexpr is_shim() {
    return geography::is_shim_tile(x, y);
  }

  template <int Dim>
  static auto constexpr get_id() {
    static_assert(0 <= Dim && Dim <= 1,
                  "The dimension has to be between 0 and 1");
    if constexpr (Dim == 0)
      return x;
    else
      return y;
  }
};

}

using namespace cl::sycl::vendor::xilinx;


bool constexpr is_red(int x, int y) {
  return (x + y);
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

template <typename ME_Array, int X, int Y>
struct tile
  : acap::tile<X, Y>
  , memory<X, Y> {
  using t = acap::tile<X, Y>;
  int v = 42;

  void run(ME_Array &a) {
    std::cout << "Hello, I am the ME tile (" << X << ',' << Y
              << ") using " << sizeof(*this) << " bytes of memory "
              << std::endl;
    if constexpr (t::is_shim()) {
      std::cout << "  and I am a shim tile ";
      if constexpr (t::is_noc())
        std::cout << "(a NoC controller)" << std::endl;
      if constexpr (t::is_pl())
        std::cout << "(a PL interface)" << std::endl;
    }
    std::cout << "Local v = " << v << std::endl;
    if constexpr (is_red(X, Y))
      std::cout << " d = " << red::d << std::endl;
    else
      std::cout << " i = " << black::i << std::endl;

    // auto neighbour_v = get_tile<get_id<0>() ^ 1, get_id<1>()>().v;
    std::cout << "Neighbour v = " << a.t1.v << std::endl;
  }
};


int main() {
  acap::me_array<tile> me;

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
