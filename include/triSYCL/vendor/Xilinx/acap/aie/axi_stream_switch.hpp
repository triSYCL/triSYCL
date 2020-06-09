#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_AXI_STREAM_SWITCH_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_AXI_STREAM_SWITCH_HPP

/** \file

    Model of an AXI stream switch

    Based on Math Engine (ME) Architecture Specification, Revision v2.01
    March 2019:

    - section "4.2 AXI-Stream Tile Interconnect"

    - section "6.5.3 Shim AXI-Stream Interconnect"

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstdint>

#include <boost/format.hpp>

#include "connection.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** An AXI stream switch parameterized with some geographic layout

    \param[in] AxiStreamGeography is the geography class used to
    instantiate the AXI stream switch.
 */
template <typename AXIStreamGeography>
class axi_stream_switch {

  /// The input communication ports for the tile
  std::array<connection::input,
             AXIStreamGeography::nb_master_port> user_in;

  /// The output communication ports for the tile
  std::array<connection::output,
             AXIStreamGeography::nb_slave_port> user_out;

public:

  /** Validate the user port number and translate it to the physical
      port number

      \param[in] user_port is the logical port number, starting to 0

      \param[in] physical_port_min is the physical lower port number to map to

      \param[in] physical_port_max is the physical higher port number
      to map to

      \throws ::trisycl::runtime_error if the port number is invalid

      \return the physical port number in the switch corresponding to
      the logical port
  */
  static auto inline translate_port = [] (int user_port,
                                          auto physical_port_min,
                                          auto physical_port_max,
                                          const auto& error_message) {
    // Cast to int since the physical port might be the enum types
    auto port_min = static_cast<int>(physical_port_min);
    auto port_max = static_cast<int>(physical_port_max);
    auto last_user_port = port_max - port_min;
    if (user_port < 0 || user_port > last_user_port)
      throw ::trisycl::runtime_error {
        (boost::format {
          "%1%: %2% is not a valid port number between 0 and %3%" }
           % error_message % user_port % last_user_port).str() };
    return port_min + user_port;
  };


  /** Access the input connection behind an input port

      \param[in] p is the input port number

      \throws ::trisycl::runtime_error if the port number is invalid
  */
  auto &in_connection(int p) {
    return user_in[p];
  }


  /** Access the output connection behind an output port

      \param[in] p is the output port number

      \throws ::trisycl::runtime_error if the port number is invalid
  */
  auto &out_connection(int p) {
    return user_out[p];
  }


  using data_type = std::uint32_t;
  static constexpr auto stream_latency = 4;

  static constexpr auto west_input_number = 4;
  static constexpr auto west_output_number = 4;
  static constexpr auto east_input_number = 4;
  static constexpr auto east_output_number = 4;
  static constexpr auto north_input_number = 4;
  static constexpr auto north_output_number = 6;
  static constexpr auto south_input_number = 8;
  static constexpr auto south_output_number = 6;
/*
  using west_input_type = std::array<stream, west_input_number>;
  using west_output_type = std::array<stream, west_output_number>;
  using east_input_type = std::array<stream, east_input_number>;
  using east_output_type = std::array<stream, east_output_number>;
  using north_input_type = std::array<stream, north_input_number>;
  using north_output_type = std::array<stream, north_output_number>;
  using south_input_type = std::array<stream, south_input_number>;
  using south_output_type = std::array<stream, south_output_number>;
*/
  struct input_port {
    ::trisycl::sycl_2_2::static_pipe<data_type, stream_latency> stream;
    bool enabled;
  };

  struct output_port {
    ::trisycl::sycl_2_2::static_pipe<data_type, stream_latency> stream;
  };

  std::array<input_port, AXIStreamGeography::nb_master_port> input_ports;
  std::array<output_port, AXIStreamGeography::nb_slave_port> output_ports;

/*
  axi_stream_switch(west_input_type wi, west_output_type wo,
                    east_input_type ei, east_output_type eo,
                    north_input_type ni, north_output_type no,
                    south_input_type si, south_output_type so)
  {}
*/

  ///
  axi_stream_switch() = default;


  auto input(typename AXIStreamGeography::master_port_layout p) {
    return input_ports[static_cast<int>(p)].stream.template
      get_access<access::mode::write, access::target::blocking_pipe>();
  }


  auto output(typename AXIStreamGeography::master_port_layout p) {
    // Lie to implement some "routing" for now
//    return output_ports[static_cast<int>(p)].stream
//      .get_access<access::mode::read, access::target::blocking_pipe>();
    return input_ports[static_cast<int>(p)].stream.template
      get_access<access::mode::read, access::target::blocking_pipe>();
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_AXI_STREAM_SWITCH_HPP
