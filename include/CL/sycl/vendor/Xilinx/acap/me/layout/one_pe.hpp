#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LAYOUT_ONE_PE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LAYOUT_ONE_PE_HPP

/** \file Layout for a AI Engine array with 1 PE only

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "full.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie::layout {

/** Describe the layout of a tiny AI Engine array with only 1 PE
 */
struct one_pe : full {
  static auto constexpr x_max = 0;
  static auto constexpr y_max = 0;
};

}
/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LAYOUT_ONE_PE_HPP
