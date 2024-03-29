#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_SIZE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_SIZE_HPP

/** \file

    Flexible layout for a AI Engine array with any 2D size

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "full.hpp"

// Just to include the namespace once in Doxygen documentation
/// \ingroup aie
namespace trisycl::vendor::xilinx::acap::aie::layout {

/// \ingroup aie
/// @{

/** Fully specify the size of an AI Engine layout

    \param X_size is the number of tile horizontally

    \param Y_size is the number of tile vertically
*/
template <int X_size, int Y_size>
struct size : full {
  static_assert(X_size >= 1 && Y_size >= 1, "Sorry, "
                "we do not yet support antimatter or black matter cores :-)");
  /// Maximum horizontal coordinate
  static auto constexpr x_max = X_size - 1;

  /// Maximum vertical coordinate
  static auto constexpr y_max = Y_size - 1;
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_LAYOUT_SIZE_HPP
