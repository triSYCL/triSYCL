/* Experimenting with processor arrays such as ACAP/MathEngine

   2017-09-18--22-Khronos_F2F_Chicago-Xilinx/2017-09-19--20-Xilinx-SYCL-Next.pdf
   Based on Math Engine (ME) Architecture Specification,Revision v1.4
   March 2018
*/

#include <iostream>
#include <type_traits>
#include <boost/hana.hpp>

namespace cl::sycl::vendor::xilinx::acap {

/** Some geographic information about the array
 */
template <typename Layout>
struct me_geography
  : Layout {
  using layout = Layout;

  /// A tuple with the coordinate tuples of all the tiles
  static auto constexpr tile_coordinates = boost::hana::cartesian_product(
    boost::hana::make_tuple(
        boost::hana::range_c<int, layout::x_min, layout::x_max + 1>
      , boost::hana::range_c<int, layout::y_min, layout::y_max + 1>
                            )
                                                                          );
  /// Generate a tuple of tileable objects
  template <template <int X, int Y> typename Tileable>
  static auto generate_tiles() {
    return boost::hana::transform(
        tile_coordinates
      , [] (auto coord) {
          return Tileable<boost::hana::at_c<0>(coord),
                          boost::hana::at_c<1>(coord)> {};
        }
                                  );
  }

  static bool constexpr is_x_valid(int x) {
    return layout::x_min <= x && x <= layout::x_max;
  }

  static bool constexpr is_y_valid(int y) {
    return layout::y_min <= y && y <= layout::y_max;
  }

  static bool constexpr is_shim_tile(int x, int y) {
    // It could be more optimized, but like that it is clearer
    return layout::is_noc_tile(x, y) || layout::is_pl_tile(x, y);
  }
};


/** The MathEngine array structure
 */
template <typename Layout,
          template <typename Geography,
                    typename ME_Array,
                    int X,
                    int Y> typename Tile>
struct me_array {

  using geography = me_geography<Layout>;

  template <int X, int Y>
  using tileable_tile = Tile<geography, me_array, X, Y>;
  /// All the tiles of the ME array.
  /// Unfortunately it is not possible to use auto here...
  decltype(geography::template generate_tiles<tileable_tile>()) tiles =
    geography::template generate_tiles<tileable_tile>();

#if 0
  template <int X, int Y>
  auto get_tile() {
    return boost::hana::find_if(
        tiles
      , [] (auto& tile) {
          return true;// tile.x == x && tile.y == y;
        }
                                );
  }
#endif
  void run() {
    boost::hana::for_each(tiles, [&] (auto& t) { t.run(*this); });

    std::cout << "Total size of the tiles: " << sizeof(tiles)
              << " bytes." << std::endl;
  }
};


/** The MathEngine tile infrastructure
 */
template <typename Geography, typename ME_Array, int X, int Y>
struct tile {
  // The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  using geography = Geography;

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


/// A small MathEngine program

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

template <typename Geography, typename ME_Array, int X, int Y>
struct tile
  : acap::tile<Geography, ME_Array, X, Y>
  , memory<X, Y> {
  using t = acap::tile<Geography, ME_Array, X, Y>;
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
    //std::cout << "Neighbour v = " << a.t1.v << std::endl;
   // auto neighbour_v = a.template get_tile<2,3>();
  }
};


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
    return y == 0 && ((0 <= x && x <= 1) || (4 <= x && x <= 5));
  }
};


/** Describe the layout of a tiny MathEngine array with only 1 PE
 */
struct me_layout_1pe : me_layout {
  static auto constexpr x_max = 0;
  static auto constexpr y_max = 0;
};

int main() {
  std::cout << std::endl << "Instantiate big MathEngine:"
            << std::endl << std::endl;
  acap::me_array<me_layout, tile> me;
  me.run();

  std::cout << std::endl << "Instantiate tiny MathEngine:"
            << std::endl << std::endl;
  acap::me_array<me_layout_1pe, tile> solitaire_me;
  solitaire_me.run();



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
