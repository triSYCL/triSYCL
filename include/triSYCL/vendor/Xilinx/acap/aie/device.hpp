#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP

/** \file

    SYCL-friendly device view of AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "geography.hpp"
#include "queue.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/
template <typename Layout>
struct device {

  /// The geography of the CGRA
  using geo = geography<Layout>;

  /// Create a queue on this device
  auto queue() {
    return vendor::xilinx::acap::aie::queue { *this };
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
