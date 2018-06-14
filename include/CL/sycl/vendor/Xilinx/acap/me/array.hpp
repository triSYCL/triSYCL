#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_ARRAY_HPP

/** \file

    Model of a MathEngine array

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>
#include <thread>

#include "geography.hpp"

namespace cl::sycl::vendor::xilinx::acap::me {

/** The MathEngine array structure
 */
template <typename Layout,
          template <typename Geography,
                    typename ME_Array,
                    int X,
                    int Y> typename Tile>
struct array {

  using geo = geography<Layout>;

  template <int X, int Y>
  using tileable_tile = Tile<geo, array, X, Y>;
  /// All the tiles of the ME array.
  /// Unfortunately it is not possible to use auto here...
  // Otherwise static inline auto
  decltype(geo::template generate_tiles<tileable_tile>()) tiles =
    geo::template generate_tiles<tileable_tile>();

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
    boost::hana::for_each(tiles, [&] (auto& t) { t.set_array(this); });

    boost::hana::for_each(tiles, [&] (auto& t) {
        t.thread = std::thread {[&] {
            TRISYCL_DUMP_T("Starting ME tile (" << t.x << ',' << t.y << ')');
            t.run(*this);
            TRISYCL_DUMP_T("Stopping ME tile (" << t.x << ',' << t.y << ')');
          }
        };
      });

    boost::hana::for_each(tiles, [&] (auto& t) {
        TRISYCL_DUMP_T("Joining ME tile (" << t.x << ',' << t.y << ')');
        t.thread.join();
        TRISYCL_DUMP_T("Joined ME tile (" << t.x << ',' << t.y << ')');
      });

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
