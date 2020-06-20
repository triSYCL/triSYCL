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
#include <string>
#include <type_traits>
#include <utility>

#include <boost/fiber/buffered_channel.hpp>
#include <boost/format.hpp>
#include "magic_enum.hpp"
#include <range/v3/all.hpp>

#include "connection.hpp"
#include "triSYCL/detail/enum.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/vendor/Xilinx/latex.hpp"

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

public:

  using axi_ss_geo = AXIStreamGeography;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;

  /** Abstract interface for a router input port

      Note that a router output is actually implemented as an input to
      some other consumer (router, core...).
   */
  struct router_port {
    /// Keep track of the AXI stream switch owning this port for debugging
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
  class router_minion : public router_port {

  public:

    /// Router ingress capacity queue
    /// \todo check with hardware team for the value
    auto static constexpr capacity = 4;

    /// Ingress packet queue
    boost::fibers::buffered_channel<axi_packet> c { capacity };

    /// The ids of the outputs port the router minion has to forward to.
    /// Used by introspection to track current routing configuration
    std::vector<axi_stream_switch::mpl> mpl_outputs;

    /** The outputs the router minion has to forward to.
        This what is used to speed up the routing itself even if the id
        could be enough */
    std::vector<std::shared_ptr<router_port>> outputs;

    /// Inherit from parent constructors
    using router_port::router_port;
    using value_type = typename router_port::value_type;

    /// To shepherd the routing fibers
    std::vector<detail::fiber_pool::future<void>> futures;

    /// Enqueue a packet on the router input
    /// \todo Clean up the API to separate data from signaling
    void write(const axi_packet &v) override {
      /// \todo separate debug from shutdown case
      TRISYCL_DUMP_T("router_minion " << this
                     << " on tile(" << router_minion::axi_ss.x_coordinate << ','
                     << router_minion::axi_ss.y_coordinate
                     << ") on fiber " << boost::this_fiber::get_id()
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
      TRISYCL_DUMP_T("router_minion " << this << " on tile("
                     << router_minion::axi_ss.x_coordinate
                     << ',' << router_minion::axi_ss.y_coordinate
                     << ") on fiber " << boost::this_fiber::get_id()
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
    void connect_to(axi_stream_switch::mpl port_dest,
                    std::shared_ptr<router_port> dest) {
      TRISYCL_DUMP_T("connect_to router_minion " << this << " on tile("
                     << router_minion::axi_ss.x_coordinate
                     << ',' << router_minion::axi_ss.y_coordinate
                     << ") on fiber " << boost::this_fiber::get_id()
                     << " to dest " << &*dest);
      mpl_outputs.push_back(port_dest);
      outputs.push_back(dest);
    }


    /// Return a vector of the master ports this router is forwarding
    const auto& get_master_port_dests() {
      return mpl_outputs;
    }


  private:

    /// Wait for the fibers to complete
    void join() {
      // Wait on all the fibers to finish
      for (auto &f : futures)
        // Get the value of the future, to get an exception if any
        f.get();
    }

  public:

    /// Create a router minion on this switch using a fiber executor
    router_minion(axi_stream_switch &axi_ss,
                  detail::fiber_pool &fiber_executor)
      : router_port { axi_ss }
    {
      /// Use a fiber as a data mover from the input queue
      futures.push_back(fiber_executor.submit([&] {
         TRISYCL_DUMP_T("router_minion " << this << " on tile("
                        << router_minion::axi_ss.x_coordinate
                        << ',' << router_minion::axi_ss.y_coordinate
                        << ") on fiber " << boost::this_fiber::get_id()
                        << " starting with buffered_channel " << &c);
          for (;;) {
            TRISYCL_DUMP_T("router_minion " << this << " on tile("
                           << router_minion::axi_ss.x_coordinate
                           << ',' << router_minion::axi_ss.y_coordinate
                           << ") on fiber " << boost::this_fiber::get_id()
                           << " reading from buffered_channel "
                           << &c << " ...");
            auto v = c.value_pop();
            if (v.shutdown_request)
              // End the execution on shutdown packet reception
              break;
            TRISYCL_DUMP_T("router_minion " << this << " on tile("
                           << router_minion::axi_ss.x_coordinate
                           << ',' << router_minion::axi_ss.y_coordinate
                           << ") on fiber " << boost::this_fiber::get_id()
                           << " routing data value " << v.data
                           << " received from buffered_channel " << &c);
            // The routing itself is a blocking write on each output
            for (auto &o : outputs) {
              TRISYCL_DUMP_T("router_minion " << this << " on tile("
                             << router_minion::axi_ss.x_coordinate
                             << ',' << router_minion::axi_ss.y_coordinate
                             << ") on fiber " << boost::this_fiber::get_id()
                             << " forwarding to router_minion " << o.get());
               o->write(v);
            }
          }
          TRISYCL_DUMP_T("router_minion " << this << " on tile("
                         << router_minion::axi_ss.x_coordinate
                         << ',' << router_minion::axi_ss.y_coordinate
                         << ") on fiber " << boost::this_fiber::get_id()
                         << " shutting down");
      }));
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
    TRISYCL_DUMP_T("connect " << this
                   << " input: " << magic_enum::enum_name(sp)
                   << " to output: " << magic_enum::enum_name(mp));
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
    rm->connect_to(mp, out_connection(mp));
  }


  // To deprecate ?
  using data_type = std::uint32_t;
  static constexpr auto stream_latency = 4;

  struct input_port {
    ::trisycl::sycl_2_2::static_pipe<data_type, stream_latency> stream;
    bool enabled;
  };

  struct output_port {
    ::trisycl::sycl_2_2::static_pipe<data_type, stream_latency> stream;
  };

  /** The input ports used to send information to the switch

      Use a shared pointer to represent the interconnect between
      switches in a shortcut way.
  */
  std::array<std::shared_ptr<router_port>,
             axi_ss_geo::nb_slave_port> input_ports;

  /** The input ports used to send information to the switch

      Use a shared pointer to represent the interconnect between
      switches in a shortcut way.
  */
  std::array<std::shared_ptr<router_port>,
             axi_ss_geo::nb_master_port> output_ports;

  /// Keep the horizontal coordinate for debug purpose
  int x_coordinate;

  /// Keep the vertical coordinate for debug purpose
  int y_coordinate;


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
     // \todo DMA

      // \todo FIFO

      // \todo CTRL
  }


  /// Start the tile infrastructure associated to the AIE device
  void start(int x, int y, detail::fiber_pool &fiber_executor) {
    x_coordinate = x;
    y_coordinate = y;
    // Add a router worker on all switch input
    for (auto &p : input_ports) {
#ifdef TRISYCL_DEBUG
      auto port_enum = static_cast<spl>(&p - std::begin(input_ports));
      auto port_name = magic_enum::enum_name(port_enum);
#endif
      TRISYCL_DUMP_T("axi_stream_switch " << this
                     << " (" << x_coordinate << ',' << y_coordinate
                     << ") start initialize input: " << port_name);
      p = std::make_shared<router_minion>(*this, fiber_executor);
    }
  }


  /** Compute the edge angle for a port name according a list of
      description associating a port name prefix and an associated
      angle in degree
   */
  int tikz_edge_angle(const std::string_view& port_name,
                      const std::initializer_list<std::tuple<std::string_view,
                                                             int>>& desc)
    const {
    for (auto& [prefix, angle] : desc)
      if (port_name.starts_with(prefix))
        return angle;
    assert(0 && "Unknown port name");
    return 0;
  }


  /// Display the internal switch connections with routing arrows
  /// perpendicular to the borders
  void display_routing_configuration(latex::context& c) const {
    for (const auto& [i, p] :  ranges::views::enumerate(input_ports)) {
      auto input_port = static_cast<spl>(i);
      auto rm = dynamic_cast<router_minion *>(p.get());
      for (auto output_port : rm->get_master_port_dests()) {
        auto input_port_name = magic_enum::enum_name(input_port);
        // Compute the angle of the starting arrow from the input
        auto input_angle = tikz_edge_angle(input_port_name,
                                           { { "east", 180 },
                                             { "north", -90 },
                                             { "west", 0 },
                                             { "me", 90 },
                                             { "south", 90 } });
        auto output_port_name = magic_enum::enum_name(output_port);
        // Compute the angle of the ending arrow to the output
        auto output_angle = tikz_edge_angle(output_port_name,
                                            { { "east", 180 },
                                              { "north", -90 },
                                              { "me", 0 },
                                              { "west", 0 },
                                              { "south", 90 } });
        c.add((boost::format { R"(
    \draw (node cs:name=S%1%)
       to [out=%2%, in=%4%] (node cs:name=M%3%);)" }
            % c.clean_node(input_port_name) % input_angle
            % c.clean_node(output_port_name) % output_angle).str());
      }
    }
  }


  /// Display a border of the AXI stream switch
  template <typename FG, typename FL, typename... EnumRanges>
  auto display_border(const std::string_view& node_attribute,
                      FG&& global_coordinate_function,
                      FL&& local_coordinate_function,
                      EnumRanges&&... enum_ranges) const {
    std::string out;
    // Index each connection displayed in order
    auto i = 0;
    boost::hana::for_each(boost::hana::make_tuple(enum_ranges...),
      [&] (auto&& r) {
        for (auto p : r) {
          auto [x, y] = local_coordinate_function(i);
          out += (boost::format { R"(
    \coordinate(%3%) at %2%;
    \node[%1%] at %2% {%4%};)" }
            % node_attribute
            % global_coordinate_function(x, y)
            // Make a node name from enum name prefixed with master/slave statu7s
            % latex::context::clean_node(magic_enum::enum_name(p),
                                         is_axi_master(p) ? "M" : "S")
            // Use the enum name as the port name to display
            % magic_enum::enum_name(p)).str();
          ++i;
        };
      });
    return out;
  }


  /// Compute the size of the graphics representation of the switch
  static vec<int, 2> display_size() {
    // Combine the lengths of the most packed sides,
    // + 1/+ 2 to keep room for labels on each side
    return { 1 + ranges::distance(axi_ss_geo::s_me_range)
             + ranges::distance(axi_ss_geo::m_south_range)
             + ranges::distance(axi_ss_geo::s_south_range),
             2 + ranges::distance(axi_ss_geo::m_me_range)
             + ranges::distance(axi_ss_geo::m_west_range)
             + ranges::distance(axi_ss_geo::s_west_range) };
  }


  /// Display the AXI stream switch
  template <typename GTC>
  void display(latex::context& c,
               const vec<int, 2>& core_size,
               const GTC& get_tikz_coordinate) const {
    const auto [ width, height ] = display_size();
    // Display the network interfaces on the top of the switch
    c.add(display_border("rotate=90,anchor=north west",
                         get_tikz_coordinate,
                         [&, height = height] (auto i) {
                           return std::tuple
                             { core_size.x()
                                + ranges::distance(axi_ss_geo::s_me_range) + i,
                               core_size.y() + height - 1 };
                         },
                         axi_ss_geo::m_north_range,
                         axi_ss_geo::s_north_range));
    // Display the network interfaces on the bottom of the switch
    c.add(display_border("rotate=90,anchor=north east",
                         get_tikz_coordinate,
                         [&] (auto i) {
                           return std::tuple { core_size.x() + i,
                                               core_size.y() };
                         },
                         ranges::views::reverse(axi_ss_geo::s_me_range),
                         axi_ss_geo::s_south_range,
                         axi_ss_geo::m_south_range));
    // Display the network interfaces on the left of the switch
    c.add(display_border("anchor=south east",
                         get_tikz_coordinate,
                         [&] (auto i) {
                           return std::tuple { core_size.x() - 1,
                                               core_size.y() + i + 1 };
                         },
                         ranges::views::reverse(axi_ss_geo::m_me_range),
                         axi_ss_geo::m_west_range,
                         axi_ss_geo::s_west_range));
    // Display the network interfaces on the right of the switch
    c.add(display_border("anchor=south west",
                         get_tikz_coordinate,
                         [&, width = width] (auto i) {
                           return std::tuple
                             { core_size.x() + width - 1,
                               core_size.y() + i + 1
                                + ranges::distance(axi_ss_geo::m_me_range) };
                         },
                         axi_ss_geo::s_east_range,
                         axi_ss_geo::m_east_range));

    display_routing_configuration(c);
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
