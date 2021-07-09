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

#include <functional>
#include <optional>

#include "memory_infrastructure.hpp"
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
#if !defined(__SYCL_DEVICE_ONLY__)
  /// TODO: The locking interface for CPU emulation need to be adapted to match
  /// the hardware one.

  /// Keep a reference to the memory_infrastructure hardware features
  std::optional<std::reference_wrapper<memory_infrastructure>> mi;

  /// Get an access to the a specific lock
  decltype(auto) lock(int i) {
    // value() will throw if there is some missed initialization
    return mi.value().get().lock(i);
  }

  /// Store a way to access to hardware infrastructure of the tile
  void set_memory_infrastructure(memory_infrastructure& m) { mi = m; }
#else
  auto lock(int i) { return hw_lock{hw::get_ptr_direction(this), i}; }
#endif
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_BASE_HPP
