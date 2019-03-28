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

#include "connection.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** An AXI stream switch parameterized with some geographic layout

    \param[in] AxiStreamGeography is the geography class used to
    instantiate the AXI stream switch.
 */
template <typename AXIStreamGeography>
class axi_stream_switch {

public:

  using mpl = typename AXIStreamGeography::master_port_layout;
  using spl = typename AXIStreamGeography::slave_port_layout;

  /// Number of ports usable by the programmer as input or output
  static constexpr int nb_user_ports = 2;

private:

  /// The input communication ports for the tile
  std::array<connection::input,
             AXIStreamGeography::nb_master_port> user_in;

  /// The output communication ports for the tile
  std::array<connection::output,
             AXIStreamGeography::nb_slave_port> user_out;

public:

  /** Validate the port number

      \param[in] p is the port number

      \throws cl::sycl::runtime_error if the port number is invalid
  */
  static void validate_port(int p) {
    if (p < 0 || p >= nb_user_ports)
      throw cl::sycl::runtime_error {
        (boost::format { "%1% is not a valid port number between 0 and %1%" }
         % p % (nb_user_ports - 1)).str() };
  }

  /** Access the input connection behind an input port

      \param[in] p is the input port number

      \throws cl::sycl::runtime_error if the port number is invalid
  */
  auto &in_connection(int p) {
    validate_port(p);
    return user_in[p];
  }


  /** Access the output connection behind an output port

      \param[in] p is the output port number

      \throws cl::sycl::runtime_error if the port number is invalid
  */
  auto &out_connection(int p) {
    validate_port(p);
    return user_out[p];
  }


  /** Get the input port

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not.  It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto in(int port) {
    return in_connection(port).template in<T, Target>();
  }


  /** Get the output port

      \param[in] InputT is the data type to be used in the transfers

      \param[in] Target specifies if the connection is blocking or
      not.  It is blocking by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto out(int port) {
    return out_connection(port).template out<T, Target>();
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
    cl::sycl::static_pipe<data_type, stream_latency> stream;
    bool enabled;
  };

  struct output_port {
    cl::sycl::static_pipe<data_type, stream_latency> stream;
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
