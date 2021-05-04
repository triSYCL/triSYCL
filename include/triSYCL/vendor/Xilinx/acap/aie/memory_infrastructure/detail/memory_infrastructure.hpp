#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_INFRASTRUCTURE_DETAIL_MEMORY_INFRASTRUCTURE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_INFRASTRUCTURE_DETAIL_MEMORY_INFRASTRUCTURE_HPP

/** \file

    The basic AI Engine homogeneous tile, with the common
    infrastructure to all the tiles, i.e. independent of x & y
    coordinates, but also from the tile program itself.

    This tile can be seen as the raw CGRA subdevice to run elemental
    functions.

    This is owned by the device, so for example the AXI stream switch
    configuration and packet can survive to some program changes.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <future>
#include <memory>
#include <optional>

#include "magic_enum.hpp"
#include <boost/format.hpp>
#include <range/v3/all.hpp>

#include "../../lock.hpp"

namespace trisycl::vendor::xilinx::acap::aie::detail {

/// \ingroup aie
/// @{

/** The AI Engine memory infrastructure detail common to all the tiles

*/
class memory_infrastructure {

  /// The lock unit of the memory tile
  lock_unit memory_locking_unit;

 public:
  /// Get access to a specific lock in this memory module
  auto& lock(int i) { return memory_locking_unit.lock(i); }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie::detail

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_INFRASTRUCTURE_DETAIL_MEMORY_INFRASTRUCTURE_HPP
