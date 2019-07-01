#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_SMALL_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_SMALL_HPP

/** \file

    Layout for a AI Engine array with a few PE only

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "full.hpp"

namespace trisycl::vendor::xilinx::acap::aie::layout {

/// \ingroup aie
/// @{

/** Describe the layout of a small AI Engine array with only a few PE
 */
struct small : full {
  static auto constexpr x_max = 1;
  static auto constexpr y_max = 2;
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_SMALL_HPP
