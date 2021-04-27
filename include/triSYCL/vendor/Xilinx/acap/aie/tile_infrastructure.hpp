#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP

/** \file

    The basic AI Engine homogeneous tile, with the common
    infrastructure to all the tiles, i.e. independent of x & y
    coordinates, but also from the tile program itself.

    This tile can be seen as the SYCL subdevice of the raw CGRA SYCL
    device to run elemental functions.

    This is owned by the device, so for example the AXI stream switch
    configuration and packet can survive to some program changes.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <utility>

#include "tile_infrastructure/detail/tile_infrastructure.hpp"
#include "triSYCL/device/facade/device.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param Geography is the \c geography type representing the full
    layout of the architecture
*/
template <typename Geography>
class tile_infrastructure
    : public facade::device<tile_infrastructure<Geography>,
                            detail::tile_infrastructure<Geography>> {
  /// The type encapsulating the implementation
  using dti = detail::tile_infrastructure<Geography>;

 public:
  /// The façade used to implement part of the use facing type
  using facade_t = facade::device<tile_infrastructure<Geography>, dti>;

  using geo = Geography;
  using axi_ss_geo = typename dti::axi_ss_geo;
  using mpl = typename dti::mpl;
  using spl = typename dti::spl;
  using axi_ss_t = typename dti::axi_ss_t;

  /// Make the implementation member directly accessible in this class
  using facade_t::implementation;

  /** Start the tile infrastructure associated to the AIE device

      \param[in] x is the horizontal coordinate for this tile

      \param[in] y is the vertical coordinate for this tile

      \param[in] fiber_executor is the executor used to run
      infrastructure details
  */
  tile_infrastructure(int x, int y,
                      ::trisycl::detail::fiber_pool& fiber_executor)
      : facade_t { std::make_shared<dti>(x, y, fiber_executor) } {}

  tile_infrastructure() = default;

  /// Get the horizontal coordinate
  int x() { return implementation->x(); }

  /// Get the vertical coordinate
  int y() { return implementation->y(); }

  /** Get the user input connection from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in_connection(int port) { return implementation->in_connection(port); }

  /** Get the user output connection to the AXI stream switch

      \param[in] port is port to use
  */
  auto& out_connection(int port) {
    return implementation->out_connection(port);
  }

  /** Get the user input port from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in(int port) { return implementation->in(port); }

  /** Get the user output port to the AXI stream switch

      \param[in] port is the port to use
  */
  auto& out(int port) { return implementation->out(port); }

  /** Get access to a receiver DMA

      \param[in] id specifies which DMA to access */
  auto& rx_dma(int id) { return implementation->rx_dma(id); }

  /** Get access to a transmit DMA

      \param[in] id specifies which DMA to access */
  auto& tx_dma(int id) { return implementation->tx_dma(id); }

  /** Get the input router port of the AXI stream switch

      \param p is the slave_port_layout for the stream
  */
  auto& input(spl p) { return implementation->input(p); }

  /** Get the output router port of the AXI stream switch

      \param p is the master_port_layout for the stream
  */
  auto& output(mpl p) { return implementation->output(p); }

  /// Launch a callable on this tile
  template <typename Work> auto& single_task(Work&& f) {
    implementation->single_task(std::forward<Work>(f));
    // To allow chaining commands
    return *this;
  }

  /// Wait for the execution of the callable on this tile
  auto& wait() {
    implementation->wait();
    // To allow chaining commands
    return *this;
  }

  /// Configure a connection of the core tile AXI stream switch
  auto& connect(typename geo::core_axi_stream_switch::slave_port_layout sp,
                typename geo::core_axi_stream_switch::master_port_layout mp) {
    implementation->connect(sp, mp);
    // To allow chaining commands
    return *this;
  }

  /// Compute the size of the graphics representation of the tile
  static vec<int, 2> display_size() { return dti::display_size(); }

  /// Display the tile to a LaTeX context
  void display(latex::context& c) const { implementation->display(c); }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP
