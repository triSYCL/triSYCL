#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_BASE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_BASE_HPP

/** \file

    The basic AI Engine homogeneous tile, with the common
    infrastructure to all the tiles (i.e. independent of x & y
    coordinates)

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <thread>

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.
*/
struct tile_base {
  /// The thread used to run this tile
  std::thread thread;

  /** Provide a run member function that does nothing so it is
      possible to write a minimum AI Engine program that does nothing.

      Note that even if this function is not virtual, in the common
      case a programmer implements it to specify the program executed
      by a tile
  */
  void run() {
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_BASE_HPP
