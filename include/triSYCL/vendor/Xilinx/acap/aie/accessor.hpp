#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP

/** \file

    SYCL accessor for AIE tile device

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <utility>

#include "device/detail/device.hpp"
#include "triSYCL/device/facade/device.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/
template <typename Buffer>
class accessor : public ::trisycl::accessor<typename Buffer::value_type,
                                            Buffer::rank(),
                                            access::mode::read_write,
                                            access::target::host_buffer> {

  using base = ::trisycl::accessor<typename Buffer::value_type,
                                   Buffer::rank(),
                                   access::mode::read_write,
                                   access::target::host_buffer>;
 public:

  using base::accessor;
  accessor(Buffer& b, auto& cgh)
    : base { b } {}
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ACCESSOR_HPP
