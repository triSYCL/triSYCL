#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_GEOGRAPHY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_GEOGRAPHY_HPP

/** \file Some geographic information about the MathEngine array

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/hana.hpp>

namespace cl::sycl::vendor::xilinx::acap {

/** Some geographic information about the MathEngine array
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

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_GEOGRAPHY_HPP
