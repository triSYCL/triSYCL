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
#include <memory>
#include <utility>

#include <boost/fiber/all.hpp>
#include <boost/format.hpp>

#include "connection.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/// Packet moving in AIE network-on-chip (NoC)
class axi_packet {
  /// Marker type used to signal network shutdown
  struct shutdown_t {};

public:

  /// Payload data type
  using value_type = std::uint32_t;

  /// Marker value used to construct a packet asking for shutdown
  static constexpr shutdown_t shutdown;

  /// Network payload
  value_type data;

  /// Signal end of packet stream
  bool tlast = false;

  /// Signal a router shutdown
  bool shutdown_request = false;

  /// Implicit constructor from a data value
  axi_packet(const value_type & data) : data { data } {}

  /// Construct a shutdown request packet from axi_packet::shutdown
  axi_packet(shutdown_t) : shutdown_request { true } {}

  axi_packet() = default;
};


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

  /** Abstract interface for a router input port

      Note that a router output is actually implemented as an input to
      some other consumer (router, core...).
   */
  struct router_port {
    /// Keep track of the AXI stream switch owning this port
    axi_stream_switch &axi_ss;

    /// Create a router port owned by an AXI stream switch
    router_port(axi_stream_switch &axi_ss)
      : axi_ss { axi_ss }
    {}


    /// Enqueue a packet on the router input
    void virtual write(axi_packet v) = 0;


    /** Try to enqueue a packet to input

        \return true if the packet is correctly enqueued
    */
    bool virtual try_write(axi_packet v) = 0;


    /// Nothing specific to do but need a virtual destructor to avoid slicing
    virtual ~router_port() = default;
  };


  /// A router input port with routing skills
  struct router_minion : router_port {
    /// Router ingress capacity queue
    /// \todo check with hardware team for the value
    auto static constexpr capacity = 4;

    /// Ingress packet queue
    boost::fibers::buffered_channel<axi_packet> c { capacity };

    /// Fiber used as a data mover from an input queue
    boost::fibers::fiber f
      { [&] {
          for (;;) {
            auto v = c.value_pop();
            if (v.shutdown_request)
              // End the execution on shutdown packet reception
              break;
            /* for now just write to me_0
               for (auto o : outputs) {
               o.write(v);
               } */
            this->axi_ss.output_ports[0]->write(v);
          }
        }
      };

    /// Inherit from parent constructors
    using router_port::router_port;

    /// Enqueue a packet on the router input
    void write(axi_packet v) override {
      c.push(v);
    }


    /** Try to enqueue a packet on the router input

        \return true if the packet is correctly enqueued
    */
    bool try_write(axi_packet v) override {
      return c.try_push(v) != boost::fibers::channel_op_status::full;
    }


    /// Destructor handling the correct infrastructure shutdown
    ~router_minion() override {
      // Send a special packet to shutdown the routing process
      write(axi_packet::shutdown);
      // Wait for shutdown to avoid calling std::terminate on destruction...
      f.join();
    }
  };


  /// A router input port directing to a AIE core input
  struct core_receiver : router_port {
    /// Router ingress capacity queue
    /// \todo check with hardware team for the value
    auto static constexpr capacity = 2;

    /// Payload data type
    using value_type = typename axi_packet::value_type;

    /* boost::fibers::unbuffered_channel has no try_push() function, so
       use a buffered version for now

       \todo open a GitHub issue on Boost.Fiber
    */
    boost::fibers::buffered_channel<axi_packet> c { capacity };

    /// Inherit from parent constructors
    using router_port::router_port;

    /// Enqueue a packet to the core input
    void write(axi_packet v) override {
      c.push(v);
    }


    /** Try to enqueue a packet to the core input

        \return true if the packet is correctly enqueued
    */
    bool try_write(axi_packet v) override {
      return c.try_push(v) == boost::fibers::channel_op_status::success;
    }


    /// Waiting read to a core input port
    auto read() {
      return c.value_pop().data;
    }


    /** Non-blocking read to a core input port

        \return true if the value was correctly read
    */
    bool try_read(value_type &v) {
      axi_packet p;

      if (c.try_pop(p) == boost::fibers::channel_op_status::success) {
        v = p.data;
        return true;
      }
      return false;
    }
  };


  /** Validate the user port number and translate it to the physical
      port number

      \param[in] user_port is the logical port number, starting to 0

      \param[in] physical_port_min is the physical lower port number to map to

      \param[in] physical_port_max is the physical higher port number
      to map to

      \throws ::trisycl::runtime_error if the port number is invalid

      \return the physical port number in the switch corresponding to
      the logical port

      \todo This makes sense only for core tile
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

  // To deprecate ?
  using data_type = std::uint32_t;
  static constexpr auto stream_latency = 4;

  struct input_port {
    ::trisycl::static_pipe<data_type, stream_latency> stream;
    bool enabled;
  };

  struct output_port {
    ::trisycl::static_pipe<data_type, stream_latency> stream;
  };

  /** The input ports used to send information to the switch

      Use a shared pointer to represent the interconnect between
      switches in a shortcut way.
  */
  std::array<std::shared_ptr<router_port>,
             AXIStreamGeography::nb_slave_port> input_ports;

  /** The input ports used to send information to the switch

      Use a shared pointer to represent the interconnect between
      switches in a shortcut way.
  */
  std::array<std::shared_ptr<router_port>,
             AXIStreamGeography::nb_master_port> output_ports;

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


  /// Connect the internals of the AXI stream switch
  axi_stream_switch() {
    // Add a router worker on all switch input
    for (auto &p : input_ports)
      p = std::make_shared<router_minion>(*this);

    // The ports to the AIE core
    for (auto &p : output_ports)
      p = std::make_shared<core_receiver>(*this);

    // \todo DMA

    // \todo FIFO

    // \todo CTRL
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
