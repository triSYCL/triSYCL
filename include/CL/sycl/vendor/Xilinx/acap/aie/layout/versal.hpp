#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_VERSAL_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_VERSAL_HPP

/** \file

    Layout for some Xilinx Versal products

    /proj/xbuilds/HEAD_INT_daily_latest/installs/lin64/cardano/HEAD/data/devices/S80.MEPL.json

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "full.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie::layout {

/// \ingroup aie
/// @{

#
/** Describe the layout of a tiny AI Engine with only 1 tile
 */
struct vc1902 : full {
  static auto constexpr x_max = 50;
  static auto constexpr y_max = 8;
};

/// Alias to vc1902
using xc10s80 = vc1902;

/// Alias to vc1902
using xcvc1902 = vc1902;


/// @} End the aie Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_VERSAL_HPP
