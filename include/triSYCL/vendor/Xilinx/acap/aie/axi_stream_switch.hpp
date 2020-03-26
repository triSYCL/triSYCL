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
#include <future>
#include <memory>
#include <type_traits>
#include <utility>

#include <boost/fiber/all.hpp>
#include <boost/format.hpp>

#include "connection.hpp"
#include "triSYCL/detail/enum.hpp"

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
  static const shutdown_t shutdown;

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
  /// Handle to the thread used to run the routing minions
  std::future<void> fiber_runner;

public:

  using mpl = typename AXIStreamGeography::master_port_layout;
  using spl = typename AXIStreamGeography::slave_port_layout;

  /** Abstract interface for a router input port

      Note that a router output is actually implemented as an input to
      some other consumer (router, core...).
   */
  struct router_port {
    /// Keep track of the AXI stream switch owning this port
    axi_stream_switch &axi_ss;

    using value_type = axi_packet::value_type;

    /// Create a router port owned by an AXI stream switch
    router_port(axi_stream_switch &axi_ss)
      : axi_ss { axi_ss }
    {}


    /// Enqueue a packet on the router input
    void virtual write(const axi_packet &v) = 0;


    /// Alias to enqueue a packet on the router input
    auto& operator<<(const value_type &v) {
      write(v);
      return *this;
    }


    /** Try to enqueue a packet to input

        \return true if the packet is correctly enqueued
    */
    bool virtual try_write(const axi_packet &v) = 0;

    /// Waiting read to a core input port
    value_type virtual read() = 0;


    /** Non-blocking read to a core input port

        \return true if the value was correctly read
    */
    bool virtual try_read(value_type &v) = 0;


    /// Alias to the waiting read to a core input port
    template <typename T>
    auto& operator>>(T &v) {
      v = static_cast<T>(read());
      return *this;
    }


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

    std::vector<std::shared_ptr<router_port>> outputs;
#if 0
    /// Fiber used as a data mover from an input queue
    boost::fibers::fiber f
      { /* Use explicit dispatch to have the scheduler starting right
           away instead of waiting for the join() (called only in the
           destructor...) to launch it */
       boost::fibers::launch::dispatch,
       [&] {
         TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                        << boost::this_fiber::get_id()
                        << " starting with buffered_channel " << &c);
          for (;;) {
            TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                           << boost::this_fiber::get_id()
                           << " reading from buffered_channel "
                           << &c << " ...");
            auto v = c.value_pop();
            if (v.shutdown_request)
              // End the execution on shutdown packet reception
              break;
            TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                           << boost::this_fiber::get_id()
                           << " routing data value " << v.data);
            // The routing itself is a blocking write on each output
            for (auto &o : outputs) {
              TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                             << boost::this_fiber::get_id()
                             << " to router_minion " << o.get());
               o->write(v);
            }
          }
          TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                         << boost::this_fiber::get_id()
                         << " shutting down");
        }
      };
#endif
    /// Inherit from parent constructors
    using router_port::router_port;
    using value_type = typename router_port::value_type;

    /// Enqueue a packet on the router input
    /// \todo Clean up the API to separate data from signaling
    void write(const axi_packet &v) override {
      /// \todo separate debug from shutdown case
      TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                     << boost::this_fiber::get_id()
                     << " write data value " << v.data
                     << " to buffered_channel " << &c);
      c.push(v);
    }


    /** Try to enqueue a packet on the router input

        \return true if the packet is correctly enqueued
    */
    bool try_write(const axi_packet &v) override {
      return c.try_push(v) != boost::fibers::channel_op_status::full;
    }


    /// Waiting read to a core input port
    value_type read() override {
      TRISYCL_DUMP_T("router_minion " << this << " on fiber "
                     << boost::this_fiber::get_id()
                     << " reading from buffered_channel " << &c << " ...");
      return c.value_pop().data;
    }


    /** Non-blocking read to a core input port

        \return true if the value was correctly read
    */
    bool try_read(value_type &v) override {
      axi_packet p;

      if (c.try_pop(p) == boost::fibers::channel_op_status::success) {
        v = p.data;
        return true;
      }
      return false;
    }


    /// Connect this routing input to a switch output
    void connect_to(std::shared_ptr<router_port> dest) {
      outputs.push_back(dest);
    }


    /** Wait for the fiber to complete if any

        Allow 2 different use cases: either end-user is waiting on
        completion by calling this function or otherwise the
        destructor will wait for it
    */
    void join() {
#if 0
      if (f.joinable())
        f.join();
#endif
    }


    /// Destructor handling the correct infrastructure shutdown
    ~router_minion() override {
      // Send a special packet to shutdown the routing process
      write(axi_packet::shutdown);
      // Wait for shutdown to avoid calling std::terminate on destruction...
      join();
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
    // Return the value as the requested enum port
    return static_cast<decltype(physical_port_min)>(port_min + user_port);
  };


  /** Access the input connection behind an input port

      \param[in] p is the input port number

      \throws ::trisycl::runtime_error if the port number is invalid
  */
  auto &
  in_connection(spl sp) {
    using u_t = std::underlying_type_t<decltype(sp)>;
    auto port = static_cast<u_t>(sp);
    // \todo replace by std::ssize() in the future
    if (port >=  static_cast<std::ptrdiff_t>(std::size(input_ports)))
      throw ::trisycl::runtime_error {
        (boost::format {
          "in_connection: %1% is not a valid port number between 0 and %2%" }
           % port % (std::size(input_ports) - 1)).str() };
    return input_ports[port];
  }


  /** Access the output connection behind an output port

      \param[in] p is the output port number

      \throws ::trisycl::runtime_error if the port number is invalid
  */
  auto &out_connection(mpl mp) {
    using u_t = std::underlying_type_t<decltype(mp)>;
    auto port = static_cast<u_t>(mp);
    // \todo replace by std::ssize() in the future
    if (port >= static_cast<std::ptrdiff_t>(std::size(output_ports)))
      throw ::trisycl::runtime_error {
        (boost::format {
          "in_connection: %1% is not a valid port number between 0 and %2%" }
           % port % (std::size(output_ports) - 1)).str() };
    return output_ports[port];
  }


  /** Configure a connection of the shim AXI stream switch

      \param[in] sp is the input port number

      \param[in] mp is the output port number

      \throws ::trisycl::runtime_error if the port number is invalid
  */
  void connect(spl sp, mpl mp) {
    // \todo factorize out
    using u_t = std::underlying_type_t<decltype(sp)>;
    auto port = static_cast<u_t>(sp);
    auto rm = dynamic_cast<router_minion *>(in_connection(sp).get());
    if (!rm) {
      throw ::trisycl::runtime_error {
        (boost::format {
          "connect: %1% is not a routable switch input port" }
           % port).str() };
    }
    rm->connect_to(out_connection(mp));
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


  /** Get the input router port of the AXI stream switch

      \param p is the slave_port_layout for the stream
  */
  auto & input(spl p) {
    // No index validation required because of type safety
    return input_ports[detail::underlying_value(p)];
  }


  /** Get the input router port of the AXI stream switch

      \param p is the master_port_layout for the stream
  */
  auto & output(mpl p) {
    // No index validation required because of type safety
    return output_ports[detail::underlying_value(p)];
  }


  /// Connect the internals of the AXI stream switch
  axi_stream_switch() {
    std::cout << "Starting axi_stream_switch()" << std::endl;
    fiber_runner = std::async(std::launch::async, [&] {
      // Add a router worker on all switch input
      for (auto &p : input_ports)
        p = std::make_shared<router_minion>(*this);
    std::cout << "Fibers launched" << std::endl;

      // \todo DMA

      // \todo FIFO

      // \todo CTRL

      // Wait for each fiber to complete
      for (auto &p : input_ports)
        static_cast<router_minion *>(p.get())->join();
    });
    std::cout << "axi_stream_switch() created" << std::endl;
  }


  /// Connect the internals of the AXI stream switch
  ~axi_stream_switch() {
    // Wait for everybody to finish
    fiber_runner.get();
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
