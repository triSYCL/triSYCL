#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_BASE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_BASE_HPP

/** \file

    The basic AI Engine homogeneous Memory Module, with the common
    infrastructure to all the memory modules (i.e. independent of x & y
    coordinates)

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    Section 4.5 ME Memory Module, p. 118

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include <thread>

#include "lock.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine Memory Module infrastructure of a tile common to all
    the tiles

    This allows some type erasure while accessing the common memory
    module infrastructure.
*/
struct memory_base {
  /// The lock unit of the memory tile
  lock_unit memory_locking_unit;

  /// Get an access to the a specific lock
  auto &lock(int i) {
    return memory_locking_unit.lock(i);
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_BASE_HPP
