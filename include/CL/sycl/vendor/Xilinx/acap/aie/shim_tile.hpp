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

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine shim tile infrastructure

    \param AIE is the type representing the full CGRA with the
    programs and memory contents

    \todo Assume only AIE Shim PL tiles for now
*/
template <typename AIE>
class shim_tile
  : public axi_stream_switch<typename AIE::geo::shim_axi_stream_switch> {
  using axi_ss_geo = typename AIE::geo::shim_axi_stream_switch;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;
  using base = axi_stream_switch<axi_ss_geo>;

  /** Map the input BLI id/port to the shim AXI stream switch port

      \param[in] port is the BLI id/port to use
  */
  static auto translate_input_port(int port) {
    return base::translate_port(port, mpl::south_0, mpl::south_last,
                                "The BLI input port is out of range");
  }


  /** Map the input BLI id/port to the shim AXI stream switch port

      \param[in] port is the BLI id/port to use
  */
  static auto translate_output_port(int port) {
    return base::translate_port(port, spl::south_0, spl::south_last,
                                "The BLI output port is out of range");
  }

public:

  /** Get the input port from the shim AXI stream switch

      \param[in] T is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto in(int port) {
    return base::in_connection(port).template in<T, Target>();
  }


  /** Get the output port to the shim AXI stream switch

      \param[in] T is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto out(int port) {
    return base::out_connection(port).template out<T, Target>();
  }


  /** Get the BLI input connection from the shim tile

      \param[in] port is the BLI id/port to use
  */
  auto& bli_in_connection(int port) {
    return base::in_connection(translate_input_port(port));
  }


  /** Get the BLI output connection from the shim tile

      \param[in] port is the BLI id/port to use
  */
  auto& bli_out_connection(int port) {
    return base::out_connection(translate_output_port(port));
  }


  /** Get the BLI (Boundary Logic Interface) input port to the PL
      (Programmable Logic in the FPGA) to read data from

      \param[in] port is the BLI id/port to use

      \param[in] T is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <int port,
            typename T,
            access::target target = access::target::blocking_pipe>
  auto bli_in() {
    return bli_in_connection(port).template in<T, target>();
  }


  /** Get the BLI (Boundary Logic Interface) output port to the PL
      (Programmable Logic in the FPGA) to write data to

      \param[in] port is the BLI id/port to use

      \param[in] T is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <int port,
            typename T,
            access::target target = access::target::blocking_pipe>
  auto bli_out() {
    return bli_out_connection(port).template out<T, target>();
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
