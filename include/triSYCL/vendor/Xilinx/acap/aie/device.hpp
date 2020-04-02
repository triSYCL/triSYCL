#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP

/** \file

    SYCL-friendly device view of AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

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

  /** A shortcut naming for the various ports used for communication
      in the device */
  using ssp = typename geo::shim_axi_stream_switch::slave_port_layout;
  using smp = typename geo::shim_axi_stream_switch::master_port_layout;
  using csp = typename geo::core_axi_stream_switch::slave_port_layout;
  using cmp = typename geo::core_axi_stream_switch::master_port_layout;

  /// The cascade stream infrastructure of the CGRA
  cascade_stream<geo> cs;

  /// A fiber pool executor to run the infrastructure powered by
  /// TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS std::thread
  detail::fiber_pool fiber_executor
    // { 1, detail::fiber_pool::sched::round_robin, false };
    { TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS,
      detail::fiber_pool::sched::work_stealing, false };

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


  /** Apply a function for each tile of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F>
  void for_each_tile(F f) {
    for_each_tile_index([&] (auto x, auto y) { f(ti[y][x]); });
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


#if 0
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
#endif

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


  /** Connect the ports of 2 tiles or shims together with a switched
      circuit

      \param[in] T is the type of the data to be transferred

      \param[in] SrcPort is the type of the source port, such as
      port::tile or port::shim

      \param[in] DstPort is the type of the destination port, such as
      port::tile or port::shim

      \throws trisycl::runtime_error if some coordinates or port
      numbers are invalid

      \todo Refactor, make the difference between user & physical ports
  */
  template <typename T, typename SrcPort, typename DstPort>
  void connect(SrcPort src, DstPort dst) {
    /// \todo move this into a factory
    connection c { ::trisycl::static_pipe<T, 4> {} };
    constexpr bool valid_src = std::is_same_v<SrcPort, port::tile>
      || std::is_same_v<SrcPort, port::shim>;
    static_assert(valid_src,
                  "SrcPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<SrcPort, port::tile>) {
       tile(src.x, src.y).out_connection(src.port) = c.out();
    }
    else if constexpr(std::is_same_v<SrcPort, port::shim>) {
       shim(src.x).bli_out_connection(src.port) = c.out();
    }
    constexpr bool valid_dst = std::is_same_v<DstPort, port::tile>
      || std::is_same_v<DstPort, port::shim>;
    static_assert(valid_dst,
                  "DstPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<DstPort, port::tile>) {
      tile(dst.x, dst.y).in_connection(dst.port) = c.in();
    }
    else if constexpr(std::is_same_v<DstPort, port::shim>) {
      shim(dst.x).bli_in_connection(dst.port) = c.in();
    }
  }


  /// Build some device level infrastructure
  device() {
    // Initialize all the tiles with their network connections first
    for_each_tile_index([&] (auto x, auto y) {
      // Start the tile infrastructure
      tile(x, y).start(fiber_executor);
    });
    // Only then we can connect the inter core tile NoC
    for_each_tile_index([&] (auto x, auto y) {
      // No CTAD yet with Boost::Hana and Clang++-10 (but works with g++-9)
      auto noc = boost::hana::make_tuple(
          // Connection topology of the NoC towards East of the switches
          std::tuple { 1, 0,
                       cmp::east_0, cmp::east_last,
                       csp::west_0, csp::west_last }
          // Connection topology of the NoC towards West of the switches
        , std::tuple { -1, 0,
                       cmp::west_0, cmp::west_last,
                       csp::east_0, csp::east_last }
          // Connection topology of the NoC towards North of the switches
        , std::tuple { 0, 1,
                       cmp::north_0, cmp::north_last,
                       csp::south_0, csp::south_last }
          // Connection topology of the NoC towards South of the switches
        , std::tuple { 0, -1,
                       cmp::south_0, cmp::south_last,
                       csp::north_0, csp::north_last }
        );
      boost::hana::for_each(noc, [&] (auto connections) {
        auto [ dx, dy, output_start, output_last, input_start, input_last ] =
          connections;
        if (geo::is_x_y_valid(x + dx, y + dy))
          for (auto [o, i] : ranges::views::zip
                 (views::enum_type(output_start, output_last),
                  views::enum_type(input_start, input_last)))
            tile(x, y).output(o) = tile(x + dx, y + dy).input(i);
      });
    });
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
