#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_HPP

/** \file

    Model of an AI Engine array

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>
#include <thread>

#include "cascade_stream.hpp"
#include "connection.hpp"
#include "geography.hpp"
#include "memory.hpp"
#include "shim_tile.hpp"
#include "tile.hpp"

/// \ingroup acap
///  @{

/** \defgroup aie AI Engine CGRA

    Extensions to support explicit AI Engine system-wide programming in C++
    @{
*/

namespace cl::sycl::vendor::xilinx::acap::aie {

/** Define an AI Engine CGRA with its code and memory per core

    \param Layout is the layout description of the machine to
    instantiate with the physical size

    \param Tile is the description of the program tiles to
    instantiate. By default each tile will run an empty program.

    \param Memory is the description of the machine memory modules. By
    default the machine has empty memory modules.
*/
template <typename Layout,
          template <typename AIE,
                    int X,
                    int Y> typename Tile = acap::aie::tile,
          template <typename AIE,
                    int X,
                    int Y> typename Memory = acap::aie::memory>
struct array {

  /// The geography of the CGRA
  using geo = geography<Layout>;

  /// The cascade stream infrastructure of the CGRA
  cascade_stream<geo> cs;

  /// Type describing all the memory modules of the CGRA
  template <int X, int Y>
  using tileable_memory = Memory<array, X, Y>;

  /** The tiled memory modules of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_memory>())
  memory_modules = geo::template generate_tiles<tileable_memory>();

  /// Type describing the programs of all the cores in the CGRA
  template <int X, int Y>
  using tileable_tile = Tile<array, X, Y>;

  /** The tiled programs of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_tile>()) tiles =
    geo::template generate_tiles<tileable_tile>();

  connection::input in[geo::y_size][geo::x_size][2];
  connection::output out[geo::y_size][geo::x_size][2];

  template <typename T>
  void connect(std::pair<int, int> source, int src_port,
               std::pair<int, int> dest, int dst_port) {
    connection c { cl::sycl::static_pipe<T, 4> {} };
    auto i = c.in();
    auto o = c.out();
    {
      auto [x, y] = source;
      out[y][x][src_port] = o;
    }
    {
      auto [x, y] = dest;
      in[y][x][dst_port] = i;
    }
  }


  /// Get a memory module by its linear id
  template <int LinearId>
  auto &get_memory_module() {
    return boost::hana::at_c<LinearId>(memory_modules);
  }

  /** Launch the programs of all the tiles of the CGRA in their own
      CPU thread and wait for their completion.

      This is the main member function to use to launch the execution.
  */
  void run() {
    // First inform each tile about their CGRA owner
    boost::hana::for_each(tiles, [&] (auto& t) {
        t.set_array(this);
      });

    // Start each tile program in its own CPU thread
    boost::hana::for_each(tiles, [&] (auto& t) {
        t.thread = std::thread {[&] {
            TRISYCL_DUMP_T("Starting ME tile (" << t.x << ',' << t.y
                           << ") linear id = " << t.get_linear_id());
            t.run();
            TRISYCL_DUMP_T("Stopping ME tile (" << t.x << ',' << t.y << ')');
          }
        };
      });

    // Wait for the end of the execution of each tile
    boost::hana::for_each(tiles, [&] (auto& t) {
        TRISYCL_DUMP_T("Joining ME tile (" << t.x << ',' << t.y << ')');
        t.thread.join();
        TRISYCL_DUMP_T("Joined ME tile (" << t.x << ',' << t.y << ')');
      });

    std::cout << "Total size of the own memory of all the tiles: "
              << sizeof(tiles) << " bytes." << std::endl;
  }
};

/// @} End the aie Doxygen group
/// @} End the acap Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_HPP
