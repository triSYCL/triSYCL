#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_MEMORY_HPP

/** \file The basic MathEngine Memory Module

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    Section 4.5 ME Memory Module, p. 118

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include <thread>

namespace cl::sycl::vendor::xilinx::acap::me {

/** The MathEngine MathEngine Memory Module infrastructure
 */
template <typename Geography, typename ME_Array, int X, int Y>
struct memory {
  /// The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  using geo = Geography;

  /// \todo Is this required
  /// Keep a reference to the array owning this tile
  ME_Array *me_array;

  void set_array(ME_Array *array) {
    me_array = array;
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_MEMORY_HPP
