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

#include "cascade_stream.hpp"
#include "geography.hpp"
#include "memory.hpp"
#include "tile.hpp"

namespace cl::sycl::vendor::xilinx::acap::me {

/** The MathEngine array structure

    \param Layout is the layout description of the machine to
    instantiate with the physical size

    \param Tile is the description of the program tiles to
    instantiate. By default each tile will run an empty program.

    \param Memory is the description of the machine memory modules. By
    default the machine has empty memory modules.
*/
template <typename Layout,
          template <typename ME_Array,
                    int X,
                    int Y> typename Tile = acap::me::tile,
          template <typename ME_Array,
                    int X,
                    int Y> typename Memory = acap::me::memory>
struct array {

  using geo = geography<Layout>;

  /// The cascade stream infrastructure
  cascade_stream<geo> cs;

  template <int X, int Y>
  using tileable_memory = Memory<array, X, Y>;

  /// All the memory module of the ME array.
  using memory_t = decltype(geo::template generate_tiles<tileable_memory>());

  /// Unfortunately it is not possible to use auto here...
  // Otherwise static inline auto
  memory_t memory_modules = geo::template generate_tiles<tileable_memory>();

  template <int X, int Y>
  using tileable_tile = Tile<array, X, Y>;
  /// All the tiles of the ME array.
  /// Unfortunately it is not possible to use auto here...
  // Otherwise static inline auto
  decltype(geo::template generate_tiles<tileable_tile>()) tiles =
    geo::template generate_tiles<tileable_tile>();


  /// Get a memory module by its linear id
  template <int LinearId>
  auto &get_memory_module() {
    return boost::hana::at_c<LinearId>(memory_modules);
  }

  void run() {
    boost::hana::for_each(tiles, [&] (auto& t) {
        t.set_array(this);
      });

    boost::hana::for_each(tiles, [&] (auto& t) {
        t.thread = std::thread {[&] {
            TRISYCL_DUMP_T("Starting ME tile (" << t.x << ',' << t.y
                           << ") linear id = " << t.get_linear_id());
            t.run();
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
