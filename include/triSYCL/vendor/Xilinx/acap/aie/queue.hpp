#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP

/** \file

    SYCL-friendly queue for AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "array.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine minimal queue

    \param AIEDevice is the AIE device targeted by the queue
*/
template <typename AIEDevice>
struct queue {
  AIEDevice aie_d;
  using geo = typename AIEDevice::geo;
  using layout = typename geo::layout;

  queue(AIEDevice &d) : aie_d { d } {}


  /** Launch a AIE execution on this queue

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename AIE,
                      int X,
                      int Y> typename Tile = acap::aie::tile,
            template <typename AIE,
                      int X,
                      int Y> typename Memory = acap::aie::memory>
  auto submit() {
    return std::async(std::launch::async,
                      [] {
                        array<layout, Tile, Memory> {}.run();
                      });
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP
