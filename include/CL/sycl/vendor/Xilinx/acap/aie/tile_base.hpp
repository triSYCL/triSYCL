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

#include <boost/format.hpp>

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
class tile_base {
  /// For the aie::array class to play with our thread
  friend AIE;

  using axi_ss_geo = typename AIE::geo::core_axi_stream_switch;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;

public:

  /// The AXI stream switch of the core tile
  axi_stream_switch<axi_ss_geo> axi_ss;

private:

  /// The thread used to run this tile
  std::thread thread;

  /// Keep a reference to the array owning this tile
  AIE *aie_array;

  /** Validate the user port number and translate it to the physical
      port number

      \param[in] StreamLayout is the port layout description defined
      in the geography (typically \c master_port_layout or \c
      slave_port_layout) providing the me_0 and me_last physical port
      numbers

      \param[in] user_port is the user port number, starting to 0

      \throws cl::sycl::runtime_error if the port number is invalid

      \return the physical port number in the switch
  */
  template <typename StreamLayout>
  static auto translate_port(int user_port) {
    constexpr auto port_min = static_cast<int>(StreamLayout::me_0);
    constexpr auto port_max = static_cast<int>(StreamLayout::me_last);
    constexpr auto last_user_port = port_max - port_min;
    if (user_port < 0 || user_port > last_user_port)
      throw cl::sycl::runtime_error {
        (boost::format { "%1% is not a valid port number between 0 and %2%" }
           % user_port % last_user_port).str() };
    return port_min + user_port;
  }

public:

  /** Provide a run member function that does nothing so it is
      possible to write a minimum AI Engine program that does nothing.

      Note that even if this function is not virtual, in the common
      case a programmer implements it to specify the program executed
      by a tile
  */
  void run() {
  }


  /** Get the user input port from the AXI stream switch

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto in(int port) {
    return axi_ss.in_connection(translate_port<mpl>(port))
      .template in<T, Target>();
  }


  /** Get the user output port to the AXI stream switch

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not. It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto out(int port) {
    return axi_ss.out_connection(translate_port<spl>(port))
      .template out<T, Target>();
  }


  /// Store a way to access to the owner CGRA
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
