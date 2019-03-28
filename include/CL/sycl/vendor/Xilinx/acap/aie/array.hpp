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
#include <type_traits>

#include "cascade_stream.hpp"
#include "connection.hpp"
#include "geography.hpp"
#include "memory.hpp"
#include "memory_base.hpp"
#include "shim_tile.hpp"
#include "tile.hpp"
#include "tile_base.hpp"

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

  /** Keep track of all the tiled memory modules as a type-erased
      memory_modules_base type to have a simpler access to the basic
      position-independent memory module features */
  memory_base *memory_modules_bases[geo::y_size][geo::x_size];

  /// Type describing the programs of all the cores in the CGRA
  template <int X, int Y>
  using tileable_tile = Tile<array, X, Y>;

  /** The tiled programs of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_tile>()) tiles =
    geo::template generate_tiles<tileable_tile>();

  /** Keep track of all the tiles as a type-erased tile_base type to
      have a simpler access to the basic position-independent tile
      features */
  tile_base<array> *tile_bases[geo::y_size][geo::x_size];

  /** The shim tiles on the lower row of the tile array

      For now we consider only homogeneous shim tiles.
  */
  shim_tile<typename geo::shim_axi_stream_switch> shims[geo::x_size];


  /** Access to the common infrastructure part of a memory module

      \param[in] x is the horizontal memory module coordinate

      \param[in] y is the vertical memory module coordinate
  */
  memory_base &memory_module(int x, int y) {
    geo::validate_x_y(x, y);
    return *memory_modules_bases[y][x];
  }


  /** Access to a heterogeneous memory module by its linear id

      \param[in] LinearId is the linear id
  */
  template <int LinearId>
  auto &memory_module() {
    return boost::hana::at_c<LinearId>(memory_modules);
  }


  /** Access to a heterogeneous memory module by its coordinates

      \param[in] X is the horizontal memory module coordinate

      \param[in] Y is the vertical memory module coordinate
  */
  template <int X, int Y>
  auto &memory_module() {
    return memory_module<geo::linear_id(X, Y)>();
  }


  /** Iterate on all the memory module bases of the AIE in an
      homogeneous way

      \param[in] F is the function to apply on each memory module base
  */
  template <typename F>
  void for_each_memory_base(F && f) {
    for (auto y = 0; y != geo::y_size; ++y)
      for (auto x = 0; x != geo::x_size; ++x)
        f(*memory_modules_bases[y][x]);
  }


  /** Access to the common infrastructure part of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws cl::sycl::runtime_error if the coordinate is invalid
  */
  auto &tile(int x, int y) {
    geo::validate_x_y(x, y);
    return *tile_bases[y][x];
  }


  /** Access to a heterogeneous tile by linear id

      \param[in] LinearId is the linear id
  */
  template <int LinearId>
  auto &tile() {
    return boost::hana::at_c<LinearId>(tiles);
  }


  /** Access to a heterogeneous tile by its coordinates

      \param[in] X is the horizontal tile coordinate

      \param[in] Y is the vertical tile coordinate
  */
  template <int X, int Y>
  auto &tile() {
    return tile<geo::linear_id(X, Y)>();
  }


  /** Iterate on all the tile bases of the AIE in an homogeneous way

      \param[in] F is the function to apply on each tile base
  */
  template <typename F>
  void for_each_tile_base(F && f) {
    for (auto y = 0; y != geo::y_size; ++y)
      for (auto x = 0; x != geo::x_size; ++x)
        f(*tile_bases[y][x]);
  }


  /** Connect the ports of 2 tiles or shims together with a switched
      circuit

      \param[in] T is the type of the data to be transferred

      \param[in] SrcPort is the type of the source port, such as
      port::tile or port::shim

      \param[in] DstPort is the type of the destination port, such as
      port::tile or port::shim

      \throws cl::sycl::runtime_error if some coordinates or port
      numbers are invalid
  */
  template <typename T, typename SrcPort, typename DstPort>
  void connect(SrcPort src, DstPort dst) {
    /// \todo move this into a factory
    connection c { cl::sycl::static_pipe<T, 4> {} };
    constexpr bool valid_src = std::is_same_v<SrcPort, port::tile>
      || std::is_same_v<SrcPort, port::shim>;
    static_assert(valid_src,
                  "SrcPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<SrcPort, port::tile>) {
       tile(src.x, src.y).out_connection(src.port) = c.out();
    }
    else if constexpr(std::is_same_v<SrcPort, port::shim>) {
       shim(src.x).out_connection(src.port) = c.out();
    }
    constexpr bool valid_dst = std::is_same_v<DstPort, port::tile>
      || std::is_same_v<DstPort, port::shim>;
    static_assert(valid_dst,
                  "DstPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<DstPort, port::tile>) {
      tile(dst.x, dst.y).in_connection(dst.port) = c.in();
    }
    else if constexpr(std::is_same_v<DstPort, port::shim>) {
      shim(dst.x).in_connection(dst.port) = c.in();
    }
  }


  /// Create the AIE array with the tiles and memory modules
  array() {
    boost::hana::for_each(tiles, [&] (auto& t) {
        // Inform each tile about their CGRA owner
        t.set_array(this);
        // Keep track of each base tile
        tile_bases[t.y][t.x] = &t;
      });
  }


  /** Launch the programs of all the tiles of the CGRA in their own
      CPU thread and wait for their completion.

      This is the main member function to use to launch the execution.
  */
  void run() {
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


  /** Access to the shim tile

      \param[in] x is the horizontal coordinate of the shim tile

      \throws cl::sycl::runtime_error if the coordinate is invalid
  */
  auto &shim(int x) {
    geo::validate_x(x);
    return shims[x];
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
