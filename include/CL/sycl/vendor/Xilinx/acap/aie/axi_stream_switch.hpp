#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_AXI_STREAM_SWITCH_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_AXI_STREAM_SWITCH_HPP

/** \file

    Model of an AXI stream switch

    Based on Math Engine (ME) Architecture Specification, Revision v1.5
    June 2018, section "4.2 AXI-Stream Tile Interconnect".

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstdint>


namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

class axi_stream_switch {

public:

  enum class shim_axi_ss_master_port : std::int8_t {
    tile_ctrl,
    fifo_0,
    fifo_1,
    south_0,
    south_1,
    south_2,
    south_3,
    south_4,
    south_5,
    west_0,
    west_1,
    west_2,
    west_3,
    north_0,
    north_1,
    north_2,
    north_3,
    north_4,
    north_5,
    east_0,
    east_1,
    east_2,
    east_3,
    // To measure the enum
    size
  };

  enum class shim_axi_ss_slave_port : std::int8_t {
    tile_ctrl,
    fifo_0,
    fifo_1,
    south_0,
    south_1,
    south_2,
    south_3,
    south_4,
    south_5,
    south_6,
    south_7,
    west_0,
    west_1,
    west_2,
    west_3,
    north_0,
    north_1,
    north_2,
    north_3,
    east_0,
    east_1,
    east_2,
    east_3,
    shim_trace,
    // To measure the enum
    size
  };


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

  std::array<input_port,
             static_cast<int>(shim_axi_ss_slave_port::size)> input_ports;
  std::array<output_port,
             static_cast<int>(shim_axi_ss_slave_port::size)> output_ports;

/*
  axi_stream_switch(west_input_type wi, west_output_type wo,
                    east_input_type ei, east_output_type eo,
                    north_input_type ni, north_output_type no,
                    south_input_type si, south_output_type so)
  {}
*/

  ///
  axi_stream_switch() = default;


  auto input(shim_axi_ss_slave_port p) {
    return input_ports[static_cast<int>(p)].stream
      .get_access<access::mode::write, access::target::blocking_pipe>();
  }


  auto output(shim_axi_ss_master_port p) {
    // Lie to implement some "routing" for now
//    return output_ports[static_cast<int>(p)].stream
//      .get_access<access::mode::read, access::target::blocking_pipe>();
    return input_ports[static_cast<int>(p)].stream
      .get_access<access::mode::read, access::target::blocking_pipe>();
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
