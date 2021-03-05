#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP

/** \file

    SYCL-friendly device view of AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <string>
#include <type_traits>

#include <boost/format.hpp>
#include <boost/hana.hpp>
#include "magic_enum.hpp"

#include "cascade_stream.hpp"
#include "geography.hpp"
#include "memory.hpp"
#include "queue.hpp"
#include "shim_tile.hpp"
#include "tile.hpp"
#include "tile_infrastructure.hpp"

#include "triSYCL/detail/fiber_pool.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/
template <typename Layout>
struct device {

  /// The geography of the CGRA
  using geo = geography<Layout>;

  /// Naming shortcut for the core AXI stream switch
  using cass = typename geo::core_axi_stream_switch;

  /// Naming shortcut for the slave ports of the core AXI stream switch
  using csp = typename cass::slave_port_layout;

  /// Naming shortcut for the master ports of the core AXI stream switch
  using cmp = typename cass::master_port_layout;

  /// Naming shortcut for the shim AXI stream switch
  using sass = typename geo::shim_axi_stream_switch;

  /// Naming shortcut for the slave ports of the shim AXI stream switch
  using ssp = typename sass::slave_port_layout;

  /// Naming shortcut for the master ports of the shim AXI stream switch
  using smp = typename sass::master_port_layout;

  /// The cascade stream infrastructure of the CGRA
  cascade_stream<geo> cs;

  /** A fiber pool executor to run the infrastructure powered by
      TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS std::thread */
  detail::fiber_pool fiber_executor
    { 1, detail::fiber_pool::sched::round_robin, false };
  /* { TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS,
       detail::fiber_pool::sched::work_stealing, false };
  */

  /** Keep track of all the tiles as a type-erased tile_base type to
      have a simpler access to the basic position-independent tile
      features */
  tile_infrastructure<device> ti[geo::y_size][geo::x_size];


  /** Apply a function for each tile index of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F>
  void for_each_tile_index(F f) {
    for (auto [ x, y ] : ranges::views::cartesian_product
           (ranges::views::iota(0, geo::x_size),
            ranges::views::iota(0, geo::y_size)))
      f(x, y);
  };


  /** Apply a function for each tile infrastructure of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F>
  void for_each_tile(F f) {
    for_each_tile_index([&] (auto x, auto y) { f(ti[y][x]); });
  };


  /** Apply a function for each x tile index of the device

      \param f is a callable that is to be called like \c f(x) for
      each horizontal index value
  */
  template <typename F>
  void for_each_tile_x_index(F f) {
    for (auto x : ranges::views::iota(0, geo::x_size))
      f(x);
  };


  /** Apply a function for each y tile index of the device

      \param f is a callable that is to be called like \c f(x) for
      each vertical index value
  */
  template <typename F>
  void for_each_tile_y_index(F f) {
    for (auto y : ranges::views::iota(0, geo::y_size))
      f(y);
  };


  /** Access to the common infrastructure part of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto &tile(int x, int y) {
    geo::validate_x_y(x, y);
    return ti[y][x];
  }


  /** The shim tiles on the lower row of the tile array

      For now we consider only homogeneous shim tiles.
  */
  shim_tile<device> shims[geo::x_size];


  /** Access to the shim tile

      \param[in] x is the horizontal coordinate of the shim tile

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto &shim(int x) {
    geo::validate_x(x);
    return shims[x];
  }


  /// Access the cascade connections
  auto &cascade() {
    return cs;
  }


  /// Create a queue on this device
  auto queue() {
    return vendor::xilinx::acap::aie::queue { *this };
  }


  /** Create a program execution for this device

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename Device,
                      int X,
                      int Y> typename Tile = acap::aie::tile,
            template <typename Device,
                      int X,
                      int Y> typename Memory = acap::aie::memory>
  auto program() {
    return aie::program<device, Tile, Memory> { *this };
  }


  /** Shortcut to run synchronously a program execution on this queue

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename Device,
                      int X,
                      int Y> typename Tile = acap::aie::tile,
            template <typename Device,
                      int X,
                      int Y> typename Memory = acap::aie::memory>
  void run() {
    queue().template run<Tile, Memory>();
  }


  /** Connect the ports of 2 tiles or shims together with an
      hyperspace switched circuit, jumping over the underlying routing
      infrastructure.

      This is used to speed up communication in emulation and to skip
      the need of a routing planner.

      \todo To deprecate and implement in another way?

      \param[in] T is the type of the data to be transferred

      \param[in] SrcPort is the type of the source port, such as
      port::tile or port::shim

      \param[in] DstPort is the type of the destination port, such as
      port::tile or port::shim

      \throws trisycl::runtime_error if some coordinates or port
      numbers are invalid

      \todo Refactor, make the difference between user & physical ports
  */
  template <typename SrcPort, typename DstPort>
  void connect(SrcPort src, DstPort dst) {
    constexpr bool valid_src = std::is_same_v<SrcPort, port::tile>
      || std::is_same_v<SrcPort, port::shim>;
    constexpr bool valid_dst = std::is_same_v<DstPort, port::tile>
      || std::is_same_v<DstPort, port::shim>;
    static_assert(valid_dst,
                  "DstPort type should be port::tile or port::shim");
    auto channel = [&] {
      if constexpr (std::is_same_v<DstPort, port::tile>)
        return tile(dst.x, dst.y).in_connection(dst.port);
      else if constexpr (std::is_same_v<DstPort, port::shim>)
        return shim(dst.x).bli_in_connection(dst.port);
    }();
    static_assert(valid_src,
                  "SrcPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<SrcPort, port::tile>) {
       tile(src.x, src.y).out_connection(src.port) = channel;
    }
    else if constexpr (std::is_same_v<SrcPort, port::shim>) {
       shim(src.x).bli_out_connection(src.port) = channel;
    }
  }


  /// Apply a function on all the AXI stream of the neighborhood of each tile
  template <typename F>
  void for_each_tile_neighborhood(F&& f) {
    for_each_tile_index([&] (auto x, auto y) {
      // No CTAD yet with Boost::Hana and Clang++-10 (but works with g++-9)
      boost::hana::for_each(geo::core_axi_stream_switch::interconnect,
                            [&] (auto connections) {
        auto [ dx, dy, master_range, slave_range ] = connections;
        if (geo::is_x_y_valid(x + dx, y + dy))
          for (auto [o, i] : ranges::views::zip(master_range, slave_range))
            f(x, y, x + dx, y + dy, o, i);
      });
    });
  }


  /// Build some device level infrastructure
  device() {
    // Initialize all the tiles with their network connections first
    for_each_tile_index([&] (auto x, auto y) {
      // Start the tile infrastructure
      tile(x, y).start(x, y, fiber_executor);
    });
    for_each_tile_x_index([&] (auto x) {
      // Start the shim infrastructure
      shim(x).start(x, fiber_executor);
    });

    // Only then we can connect the inter-core tile AXI stream NoC
    for_each_tile_neighborhood([&] (auto x, auto y, auto nx, auto ny,
                                    auto m, auto s) {
      tile(x, y).output(m) = tile(nx, ny).input(s);
    });

    /// Make the connections around the shim tiles
    for_each_tile_x_index([&] (auto x) {
      // The connection from shim tile to core tile
      for (auto [o, i] : ranges::views::zip(sass::m_north_range,
                                            cass::s_south_range))
        shim(x).output(o) = tile(x, 0).input(i);
      // The connection from core tile to shim tile
      for (auto [o, i] : ranges::views::zip(cass::m_south_range,
                                            sass::s_north_range))
        tile(x, 0).output(o) = shim(x).input(i);
      if (!geo::is_east_column(x))
        for (auto [o, i] : ranges::views::zip(sass::m_east_range,
                                              sass::s_west_range))
          shim(x).output(o) = shim(x + 1).input(i);
      if (!geo::is_west_column(x))
        for (auto [o, i] : ranges::views::zip(sass::m_west_range,
                                              sass::s_east_range))
          shim(x).output(o) = shim(x - 1).input(i);
    });

  }


  /** Display the device layout

      \param[in] file_name is the name of the file to write the LaTeX
      drawing into or use std::cerr by default
  */
  auto display(const std::string& file_name = {}) {
    // Compute the drawing size starting with the individual tile size
    auto tile_size = tile(0, 0).display_size();
    // And expanding according to the device size. Add 1 in each
    // dimension to fit in the page because of the borders
    auto x_size = tile_size.x()*geo::x_size + 1;
    auto y_size = tile_size.y()*geo::y_size + 1;
    /// The LaTeX generation is handled by a LaTeX context
    latex::context c {{ x_size, y_size }, file_name};

    for_each_tile_index([&] (auto x, auto y) {
      tile(x, y).display(c);
    });

    // Connect each tile to its neighbors
    for_each_tile_neighborhood([&] (auto x, auto y, auto nx, auto ny,
                                    auto m, auto s) {
          c.add((boost::format { R"(
    \draw (node cs:name=TileX%1%Y%2%M%3%)
       -- (node cs:name=TileX%4%Y%5%S%6%);)" }
              % x % y % c.clean_node(magic_enum::enum_name(m))
              % nx % ny % c.clean_node(magic_enum::enum_name(s))).str());
    });

    c.display();
  }
};

/// @} End the aie Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
