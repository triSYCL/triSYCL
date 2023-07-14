#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_SHIM_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_SHIM_TILE_HPP

/** \file

    Model of a shim tile between the AI array and the rest of the circuit

    Based on Math Engine (ME) Architecture Specification Revision
    v2.01 March 2019, section "6 ME Shim Architecture".

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "axi_stream_switch.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine shim tile infrastructure

    \param AIE is the type representing the full CGRA with the
    programs and memory contents

    \todo Assume only AIE Shim PL tiles for now
*/
template <typename AIEDevice>
class shim_tile {
  using geo = typename AIEDevice::geo;
  using axi_ss_geo = typename geo::shim_axi_stream_switch;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;
  using axi_ss_t = axi_stream_switch<axi_ss_geo>;

  /// Keep the horizontal coordinate for debug purpose
  int x_coordinate;

  /// \todo Remove?
  /// Keep the vertical coordinate for debug purpose
  int y_coordinate = -1;

  /// The AXI stream switch of the core tile
  axi_ss_t axi_ss;

#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
  /// Keep track of the fiber executor
  ::trisycl::detail::fiber_pool *fe;

  /// To shepherd the working fiber
  ::trisycl::detail::fiber_pool::future<void> future_work;
#else
  /// Keep track of the std::thread execution in this tile
  std::future<void> future_work;
#endif

public:

  /// Construct the shim tile infrastructure
  shim_tile() {
    // Connect the core receivers to its AXI stream switch
    for (auto p : axi_ss_geo::m_south_range)
      output(p) = std::make_shared<port_receiver<axi_ss_t>>
        (axi_ss, "BLI_shim_receiver");
  }


    /** Start the shim tile infrastructure associated to the AIE device

      \param[in] x is the horizontal coordinate for this tile

      \param[in] fiber_executor is the executor used to run
      infrastructure details
  */
  void start(int x, ::trisycl::detail::fiber_pool &fiber_executor) {
    x_coordinate = x;
#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
    fe = &fiber_executor;
#endif
    axi_ss.start(x, -1, fiber_executor);
  }


  /// \todo Factorize some code between core tile and shim tile
  /** Get the input router port of the AXI stream switch

      \param p is the slave_port_layout for the stream
  */
  auto& input(spl p) {
    // No index validation required because of type safety
    return axi_ss.input(p);
  }


  /** Get the output router port of the AXI stream switch

      \param p is the master_port_layout for the stream
  */
  auto& output(mpl p) {
    // No index validation required because of type safety
    return axi_ss.output(p);
  }


  /** Map the input BLI id/port to the shim AXI stream switch port

      \param[in] port is the BLI id/port to use
  */
  static auto translate_input_port(int port) {
    return axi_ss_t::translate_port(port, spl::south_0, spl::south_last,
                                    "The BLI input port is out of range");
  }


  /** Map the input BLI id/port to the shim AXI stream switch port

      \param[in] port is the BLI id/port to use
  */
  static auto translate_output_port(int port) {
    return axi_ss_t::translate_port(port, mpl::south_0, mpl::south_last,
                                    "The BLI output port is out of range");
  }

public:

  /** Get the BLI input connection from the shim tile

      \param[in] port is the BLI id/port to use
  */
  auto& bli_in_connection(int port) {
    // The input is actually the output of the switch
    return axi_ss.out_connection(translate_output_port(port));
  }


  /** Get the BLI output connection to the shim tile

      \param[in] port is the BLI id/port to use
  */
  auto& bli_out_connection(int port) {
    // The output is actually the input of the switch
    return axi_ss.in_connection(translate_input_port(port));
  }


  /** Get the BLI (Boundary Logic Interface) input port from the PL
      (Programmable Logic in the FPGA) to read data from

      \param[in] port is the BLI id/port to use

  */
  auto& bli_in(int port) {
    return *bli_in_connection(port);
  }


  /** Get the BLI (Boundary Logic Interface) output port to the PL
      (Programmable Logic in the FPGA) to write data to

      \param[in] port is the BLI id/port to use
  */
  auto& bli_out(int port) {
    return *bli_out_connection(port);
  }


  /// Configure a connection of the shim tile AXI stream switch
  void connect(typename geo::shim_axi_stream_switch::slave_port_layout sp,
               typename geo::shim_axi_stream_switch::master_port_layout mp) {
    axi_ss.connect(sp, mp);
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_SHIM_TILE_HPP
