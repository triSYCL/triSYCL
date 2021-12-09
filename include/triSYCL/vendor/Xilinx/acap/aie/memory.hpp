#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_HPP

/** \file

    The basic AI Engine heterogeneous Memory Module (i.e. dependent of
    x & y coordinates)

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    Section 4.5 ME Memory Module, p. 118

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine Memory Module infrastructure of a tile

    This is the type you need to inherit from to define the content of
    a CGRA tile memory module.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents

    \param X is the horizontal coordinate of the memory module

    \param Y is the vertical coordinate of the memory module
*/
template <typename AIE //< The type representing the full CGRA
          ,
          int X //< The horizontal coordinate of the memory module
          ,
          int Y //< The vertical coordinate of the memory module
          >
struct memory {
  // TODO: should we still need this class ? It can be useful for the user to
  // mark the memory type and the runtime to detect it via std::is_base_of but
  // this is currently not done
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_MEMORY_HPP
