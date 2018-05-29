#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_HPP

/** \file

    Model of a MathEngine array

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "me_geography.hpp"

namespace cl::sycl::vendor::xilinx::acap {

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
  // Otherwise static inline auto
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

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_HPP
