#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_HPP

/** \file The basic AI Engine Memory Module

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    Section 4.5 ME Memory Module, p. 118

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include <thread>

#include "lock.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/** The AI Engine Memory Module infrastructure
 */
template <typename AIE, int X, int Y>
struct memory {
  /// The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  /// The lock unit of the memory tile
  lock_unit lu;
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_HPP
