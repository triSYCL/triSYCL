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

#include "axi_stream_switch.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents
*/
template <typename AIE>
struct tile_base {
  /// The AXI stream switch of the core tile
  axi_stream_switch<typename AIE::geo::core_axi_stream_switch> axi_ss;

  /// The thread used to run this tile
  std::thread thread;

  /// Keep a reference to the array owning this tile
  AIE *aie_array;

  /** Provide a run member function that does nothing so it is
      possible to write a minimum AI Engine program that does nothing.

      Note that even if this function is not virtual, in the common
      case a programmer implements it to specify the program executed
      by a tile
  */
  void run() {
  }


  /** Get the input port from the AXI stream switch

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto in(int port) {
    return axi_ss.in_connection(port).template in<T, Target>();
  }


  /** Get the output port to the AXI stream switch

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto out(int port) {
    return axi_ss.out_connection(port).template out<T, Target>();
  }


  // Store a way to access to the owner CGRA
  void set_array(AIE *array) {
    aie_array = array;
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
