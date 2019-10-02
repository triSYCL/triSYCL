#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP

/** \file

    The basic AI Engine homogeneous tile, with the common
    infrastructure to all the tiles, i.e. independent of x & y
    coordinates, but also from the tile program itself.

    For example the AXI stream switch configuration and packet can
    survive to some program changes.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <future>

#include <boost/format.hpp>

#include "axi_stream_switch.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents
*/
template <typename AIEDevice>
class tile_infrastructure  {
  using axi_ss_geo = typename AIEDevice::geo::core_axi_stream_switch;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;
  using axi_ss_t = axi_stream_switch<axi_ss_geo>;

  /// The AXI stream switch of the core tile
  axi_ss_t axi_ss;

  /// Keep track of execution in this tile
  std::future<void> work;

private:

  /** Map the user input port number to the AXI stream switch port

      \param[in] port is the user port to use
  */
  static auto translate_input_port(int port) {
    return axi_ss_t::translate_port(port, mpl::me_0, mpl::me_last,
                                    "The core input port is out of range");
  }


  /** Map the user output port number to the AXI stream switch port

      \param[in] port is the BLI id/port to use
  */
  static auto translate_output_port(int port) {
    return axi_ss_t::translate_port(port, spl::me_0, spl::me_last,
                                    "The core output port is out of range");
  }

public:

  /** Get the user input connection from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in_connection(int port) {
    return axi_ss.in_connection(translate_input_port(port));
  }


  /** Get the user output connection to the AXI stream switch

      \param[in] port is port to use
  */
  auto& out_connection(int port) {
    return axi_ss.out_connection(translate_output_port(port));
  }


  /** Get the user input port from the AXI stream switch

      \param[in] T is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default

      \param[in] port is the port to use
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto in(int port) {
    return axi_ss.in_connection(translate_input_port(port))
      .template in<T, Target>();
  }


  /** Get the user output port to the AXI stream switch

      \param[in] T is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default

      \param[in] port is the port to use
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto out(int port) {
    return axi_ss.out_connection(translate_output_port(port))
      .template out<T, Target>();
  }


  /// Submit a callable on this tile
  template <typename Work>
  void submit(Work &&f) {
    work = std::async(std::launch::async,
                      std::forward<Work>(f));
  }


  /// Wait for the execution of the callable on this tile
  void wait() {
    work.get();
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP
