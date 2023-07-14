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

#include <boost/fiber/all.hpp>
#include <boost/format.hpp>
#include "magic_enum.hpp"
#include <range/v3/all.hpp>

#include "connection.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/enum.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/vendor/Xilinx/latex.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** An AXI stream switch parameterized with some geographic layout

    \param[in] AxiStreamGeography is the geography class used to
    instantiate the AXI stream switch.
 */
template <typename AXIStreamGeography>
class axi_stream_switch
  : ::trisycl::detail::debug<axi_stream_switch <AXIStreamGeography>> {

public:

  using axi_ss_geo = AXIStreamGeography;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;

  /// A router input port with routing skills
  class router_minion : public communicator_port {
public  :
    /// Router ingress capacity queue
    const int capacity;

    /// Ingress packet queue
    boost::fibers::buffered_channel<axi_packet> c
      { static_cast<std::size_t>(capacity) };

    /// The ids of the outputs port the router minion has to forward to.
    /// Used by introspection to track current routing configuration
    std::vector<axi_stream_switch::mpl> mpl_outputs;

    /** The outputs the router minion has to forward to.
        This what is used to speed up the routing itself even if the id
        could be enough */
    std::vector<std::shared_ptr<communicator_port>> outputs;

    /// To shepherd the routing fibers
    std::vector<::trisycl::detail::fiber_pool::future<void>> futures;

    /// Keep track of the AXI stream switch owning this port for debugging
    axi_stream_switch &axi_ss;

    /// Wait for the fibers to complete
    void join() {
      // Wait on all the fibers to finish
      for (auto &f : futures)
        // Get the value of the future, to get an exception if any
        f.get();
    }

  public:

    using value_type = axi_packet::value_type;

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
                    std::shared_ptr<communicator_port> dest) {
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


    /// Create a router minion on this switch using a fiber executor
    router_minion(axi_stream_switch &axi_ss,
                  ::trisycl::detail::fiber_pool &fiber_executor,
                  int capacity = axi_ss_geo::latency)
      : capacity { capacity }
      , axi_ss { axi_ss }
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


  /// Get the router_minion behind a communicator_port
  static router_minion *
  get_as_router_minion(communicator_port * rp) {
#ifdef NDEBUG
    // No assert mode
    auto rm = static_cast<router_minion *>(rp);
#else
    // In mode with assertion, check this is really a router_minion
    auto rm = dynamic_cast<router_minion *>(rp);
    if (!rm) {
      throw ::trisycl::runtime_error {
        "This is not a routable switch port" };
    }
#endif
    return rm;
  }


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
          "out_connection: %1% is not a valid port number between 0 and %2%" }
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
    get_as_router_minion(in_connection(sp).get())
      ->connect_to(mp, out_connection(mp));
  }

  // \todo To deprecate ?
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
  std::array<std::shared_ptr<communicator_port>,
             axi_ss_geo::nb_slave_port> input_ports;

  /** The input ports used to send information to the switch

      Use a shared pointer to represent the interconnect between
      switches in a shortcut way.
  */
  std::array<std::shared_ptr<communicator_port>,
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
    return input_ports[::trisycl::detail::underlying_value(p)];
  }


  /** Get the input router port of the AXI stream switch

      \param p is the master_port_layout for the stream
  */
  auto & output(mpl p) {
    // No index validation required because of type safety
    return output_ports[::trisycl::detail::underlying_value(p)];
  }


#if 0
  /** Map the user receive DMA port number to the AXI stream switch port

      \param[in] port is the user port to use
  */
  static auto translate_rx_dma_port(int port) {
    return axi_ss_t::translate_port(port, spl::dma_0, spl::dma_last,
                                    "The receive DMA port is out of range");
  }


  /** Map the user transmit DMA port number to the AXI stream switch port

      \param[in] port is the user port to use
  */
  static auto translate_tx_dma_port(int port) {
    return axi_ss_t::translate_port(port, mpl::dma_0, mpl::dma_last,
                                    "The transmit DMA port is out of range");
  }
#endif


  /// Connect the internals of the AXI stream switch
  axi_stream_switch() {
    // \todo DMA

    // \todo CTRL
  }


  /// Start the tile infrastructure associated to the AIE device
  void start(int x, int y, ::trisycl::detail::fiber_pool &fiber_executor) {
    x_coordinate = x;
    y_coordinate = y;
    // Add a router worker on all switch input
    for (auto& p : input_ports) {
      auto port_enum =
        magic_enum::enum_cast<spl>(std::distance(input_ports.begin(), &p));
#ifdef TRISYCL_DEBUG
      auto port_name = magic_enum::enum_name(port_enum.value());
#endif
      TRISYCL_DUMP_T("axi_stream_switch " << this
                     << " (" << x_coordinate << ',' << y_coordinate
                     << ") start initialize input: " << port_name);
      // If the port comes from the FIFO, use a FIFO-featured
      // router_minion instead
      if (ranges::find(axi_ss_geo::s_fifo_range, port_enum)
          != std::end(axi_ss_geo::s_fifo_range))
        /* A special AXI switch routing interface which provides a
           FIFO.  Since a router_minion has already some internal FIFO
           behavior, some deeper FIFO behavior can actually be
           implemented with a specific router_minion with a deeper
           FIFO internally, to avoid having 2 active components, the
           FIFO + the router_minion */
        p = std::make_shared<router_minion>(*this,
                                            fiber_executor,
                                            axi_ss_geo::fifo_depth);
      else
        p = std::make_shared<router_minion>(*this, fiber_executor);
    }
    // Since the FIFO behavior is actually implemented with a
    // router_minion with a deeper FIFO internally, the "input" of the
    // "FIFO" can be just directly connected to this specific
    // router_minion
    for (auto [o, i] : ranges::views::zip(axi_ss_geo::m_fifo_range,
                                          axi_ss_geo::s_fifo_range))
      output(o) = input(i);
  }


  /** Compute the edge angle for a port name according a list of
      description associating a port name prefix and an associated
      angle in degree
  */
  int
  tikz_edge_angle(const std::string_view& port_name,
                  const std::initializer_list<std::tuple<std::string_view,
                                                         int>>& desc) const {
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
      auto rm = get_as_router_minion(p.get());
      for (auto output_port : rm->get_master_port_dests()) {
        auto input_port_name = magic_enum::enum_name(input_port);
        // Compute the angle of the starting arrow from the input
        auto input_angle = tikz_edge_angle(input_port_name,
                                           { { "east", 180 }, ///< To the left
                                             { "north", -90 }, ///< Downwards
                                             { "west", 0 }, ///< To the right
                                             { "dma", 90 }, ///< Upwards
                                             { "fifo", 90 },
                                             { "me", 90 },
                                             { "tile_ctrl", 90 },
                                             { "south", 90 } });
        auto output_port_name = magic_enum::enum_name(output_port);
        // Compute the angle of the ending arrow to the output
        auto output_angle = tikz_edge_angle(output_port_name,
                                            { { "east", 180 }, ///< To the left
                                              { "north", -90 }, ///< Downwards
                                              { "dma", 0 }, ///< To the right
                                              { "fifo", 0 },
                                              { "me", 0 },
                                              { "tile_ctrl", 0 },
                                              { "west", 0 },
                                              { "south", 90 } }); ///< Upwards
        // Use specific color for core emission or reception
        auto color = input_port_name.starts_with("me") ? "red"
          : output_port_name.starts_with("me") ? "teal"
          : input_port_name.starts_with("dma") ? "magenta"
          : output_port_name.starts_with("dma") ? "violet" : "blue";
        c.add((boost::format { R"(
    \draw[%5%] (node cs:name=S%1%)
       to [out=%2%, in=%4%] (node cs:name=M%3%);)" }
            % c.clean_node(input_port_name) % input_angle
            % c.clean_node(output_port_name) % output_angle
            % color).str());
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
             + ranges::distance(axi_ss_geo::m_dma_range)
             + ranges::distance(axi_ss_geo::m_tile_ctrl_range)
             + ranges::distance(axi_ss_geo::m_fifo_range)
             + ranges::distance(axi_ss_geo::m_south_range)
             + ranges::distance(axi_ss_geo::s_south_range),
             2 + ranges::distance(axi_ss_geo::s_me_range)
             + ranges::distance(axi_ss_geo::s_dma_range)
             + ranges::distance(axi_ss_geo::s_tile_ctrl_range)
             + ranges::distance(axi_ss_geo::s_fifo_range)
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
    c.add(display_border
          ("rotate=90,anchor=north west", get_tikz_coordinate,
           [&, height = height] (auto i) {
             return std::tuple
               { core_size.x()
                   + ranges::distance(axi_ss_geo::s_me_range)
                   + ranges::distance(axi_ss_geo::s_dma_range)
                   + ranges::distance(axi_ss_geo::s_tile_ctrl_range)
                   + ranges::distance(axi_ss_geo::s_fifo_range) + i,
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
                         axi_ss_geo::s_dma_range,
                         axi_ss_geo::s_tile_ctrl_range,
                         axi_ss_geo::s_fifo_range,
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
                         axi_ss_geo::m_dma_range,
                         axi_ss_geo::m_tile_ctrl_range,
                         axi_ss_geo::m_fifo_range,
                         axi_ss_geo::m_west_range,
                         axi_ss_geo::s_west_range));
    // Display the network interfaces on the right of the switch
    c.add(display_border
          ("anchor=south west",
           get_tikz_coordinate,
           [&, width = width] (auto i) {
             return std::tuple
               { core_size.x() + width - 1,
                  core_size.y() + i + 1
                  + ranges::distance(axi_ss_geo::m_me_range)
                  + ranges::distance(axi_ss_geo::m_dma_range)
                  + ranges::distance(axi_ss_geo::m_tile_ctrl_range)
                  + ranges::distance(axi_ss_geo::m_fifo_range) };
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
