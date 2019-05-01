#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_SHIM_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_SHIM_TILE_HPP

/** \file

    Model of a shim tile between the AI array and the rest of the circuit

    Based on Math Engine (ME) Architecture Specification, Revision v1.5
    June 2018, section "6 ME Shim Architecture".

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
*/
template <typename AIE>
class shim_tile
  : public axi_stream_switch<typename AIE::geo::shim_axi_stream_switch> {
  using base = axi_stream_switch<typename AIE::geo::shim_axi_stream_switch>;

public:

  /** Get the input port from the AXI stream switch

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto in(int port) {
    return base::in_connection(port).template in<T, Target>();
  }


  /** Get the output port to the AXI stream switch

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto out(int port) {
    return base::out_connection(port).template out<T, Target>();
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
