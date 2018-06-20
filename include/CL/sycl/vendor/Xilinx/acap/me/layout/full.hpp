#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LAYOUT_FULL_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LAYOUT_FULL_HPP

/** \file Layout for a MathEngine array with 5*6 PE

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "full.hpp"

namespace cl::sycl::vendor::xilinx::acap::me::layout {

/** Describe the layout of the full MathEngine array

    Use some constrains from Figure 2-8 "ME Array Address Map
    Example", p. 44
*/
struct full {
  static auto constexpr x_min = 0;
  static auto constexpr x_max = 5;
  static auto constexpr y_min = 0;
  static auto constexpr y_max = 4;

  static bool constexpr is_noc_tile(int x, int y) {
    return y == 0 && 2 <= x && x <= 3;
  }

  static bool constexpr is_pl_tile(int x, int y) {
    return y == 0 && ((0 <= x && x <= 1) || (4 <= x && x <= 5));
  }
};

}
/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_LAYOUT_FULL_HPP
