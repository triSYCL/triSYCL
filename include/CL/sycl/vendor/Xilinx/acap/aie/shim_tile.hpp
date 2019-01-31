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

class shim_tile {

public:

  axi_stream_switch axi_ss;

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_SHIM_TILE_HPP
