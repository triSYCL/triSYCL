#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_INFRASTRUCTURE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_INFRASTRUCTURE_HPP

/** \file

    The basic AI Engine homogeneous memory infrastructure,
    i.e. independent of x & y coordinates.

    This is owned by a device, so for example the lock configuration
    can survive to some program changes.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <utility>

#include "memory_infrastructure/detail/memory_infrastructure.hpp"
#include "triSYCL/device/facade/device.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine memory infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    memory infrastructure.

    \todo Recycle the device_facade for now

    \param Geography is the \c geography type representing the full
    layout of the architecture
*/
class memory_infrastructure
    : public facade::device<memory_infrastructure,
                            detail::memory_infrastructure> {
  /// The type encapsulating the implementation
  using dti = detail::memory_infrastructure;

 public:
  /// The façade used to implement part of the use facing type
  using facade_t = facade::device<memory_infrastructure, dti>;

  /// Make the implementation member directly accessible in this class
  using facade_t::implementation;

  /** Start the memory infrastructure associated to the AIE device

      \param[in] x is the horizontal coordinate for this tile

      \param[in] y is the vertical coordinate for this tile
  */
  memory_infrastructure()
      : facade_t { std::make_shared<dti>() } {}

  // Forward everything to the implementation detail
  auto& operator->() { return implementation; }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_INFRASTRUCTURE_HPP
