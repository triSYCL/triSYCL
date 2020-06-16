#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP

/** \file

    The basic AI Engine homogeneous tile, with the common
    infrastructure to all the tiles, i.e. independent of x & y
    coordinates, but also from the tile program itself.

    For example the AXI stream switch configuration and packet can
    survive to some program changes.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <future>

#include <boost/format.hpp>
#include "magic_enum.hpp"
#include <range/v3/all.hpp>

#include "axi_stream_switch.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/detail/ranges.hpp"
#include "triSYCL/vendor/Xilinx/config.hpp"
#include "triSYCL/vendor/Xilinx/latex.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents
*/
template <typename AIEDevice>
class tile_infrastructure  {
  using geo = typename AIEDevice::geo;
  using axi_ss_geo = typename geo::core_axi_stream_switch;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;
  using axi_ss_t = axi_stream_switch<axi_ss_geo>;

  /// Keep the horizontal coordinate for debug purpose
  int x_coordinate;

  /// Keep the vertical coordinate for debug purpose
  int y_coordinate;

  /// The AXI stream switch of the core tile
  axi_ss_t axi_ss;

private:

#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
  /// Keep track of the fiber executor
  detail::fiber_pool *fe;

  /// To shepherd the working fiber
  detail::fiber_pool::future<void> future_work;
#else
  /// Keep track of the std::thread execution in this tile
  std::future<void> future_work;
#endif


  /** Map the user input port number to the AXI stream switch port

      \param[in] port is the user port to use
  */
  static auto translate_input_port(int port) {
    return axi_ss_t::translate_port(port, spl::me_0, spl::me_last,
                                    "The core input port is out of range");
  }


  /** Map the user output port number to the AXI stream switch port

      \param[in] port is the BLI id/port to use
  */
  static auto translate_output_port(int port) {
    return axi_ss_t::translate_port(port, mpl::me_0, mpl::me_last,
                                    "The core output port is out of range");
  }

public:

  /// A router input port directing to a AIE core input
  struct core_receiver : axi_ss_t::router_port {
    using rp = typename axi_ss_t::router_port;

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
    using axi_ss_t::router_port::router_port;

    /// Enqueue a packet ()coming from the switch) to the core input
    void write(const axi_packet &v) override {
      TRISYCL_DUMP_T("core_receiver " << this << " on tile("
                     << rp::axi_ss.x_coordinate
                     << ',' << rp::axi_ss.y_coordinate
                     << ") on fiber " << boost::this_fiber::get_id()
                     << " write data value " << v.data
                     << " to buffered_channel " << &c);
      c.push(v);
    }


    /** Try to enqueue a packet to the core input

        \return true if the packet is correctly enqueued
    */
    bool try_write(const axi_packet &v) override {
      return c.try_push(v) == boost::fibers::channel_op_status::success;
    }


    /// Waiting read by a tile program on a core input port from the switch
    value_type read() override {
      TRISYCL_DUMP_T("core_receiver " << this << " on tile("
                     << rp::axi_ss.x_coordinate
                     << ',' << rp::axi_ss.y_coordinate
                     << ") on fiber " << boost::this_fiber::get_id()
                     << " reading from buffered_channel " << &c << "...");
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
  };


  /// Construct the tile infrastructure
  tile_infrastructure() {
    // Connect the core receivers to its AXI stream switch
    for (auto p : views::enum_type(mpl::me_0, mpl::me_last))
      output(p) = std::make_shared<core_receiver>(axi_ss);
  }


  /** Start the tile infrastructure associated to the AIE device

      \param[in] x is the horizontal coordinate for this tile

      \param[in] y is the vertical coordinate for this tile

      \param[in] fiber_executor is the executor used to run
      infrastructure details
  */
  void start(int x, int y, detail::fiber_pool &fiber_executor) {
    x_coordinate = x;
    y_coordinate = y;
#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
    fe = &fiber_executor;
#endif
    axi_ss.start(x, y, fiber_executor);
  }


  /** Get the user input connection from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in_connection(int port) {
    return axi_ss.in_connection(translate_input_port(port));
  }


  /** Get the user output connection to the AXI stream switch

      \param[in] port is port to use
  */
  auto& out_connection(int port) {
    return axi_ss.out_connection(translate_output_port(port));
  }


  /** Get the user input port from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in(int port) {
    TRISYCL_DUMP_T("in(" << port << ") on tile(" << x_coordinate << ','
                   << y_coordinate << ')');
    /* The input port for the core is actually the corresponding
       output on the switch */
    return *axi_ss.out_connection(translate_output_port(port));
  }


  /** Get the user output port to the AXI stream switch

      \param[in] port is the port to use
  */
  auto& out(int port) {
    TRISYCL_DUMP_T("out(" << port << ") on tile(" << x_coordinate << ','
                   << y_coordinate << ')');
    /* The output port for the core is actually the corresponding
       input on the switch */
    return *axi_ss.in_connection(translate_input_port(port));
  }


  /** Get the input router port of the AXI stream switch

      \param p is the slave_port_layout for the stream
  */
  auto & input(spl p) {
    // No index validation required because of type safety
    return axi_ss.input(p);
  }


  /** Get the output router port of the AXI stream switch

      \param p is the master_port_layout for the stream
  */
  auto & output(mpl p) {
    // No index validation required because of type safety
    return axi_ss.output(p);
  }


  /// Submit a callable on this tile
  template <typename Work>
  void submit(Work &&f) {
    // Launch the tile program immediately on a new executor engine
#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
    future_work = fe->submit(std::forward<Work>(f));
#else
    future_work = std::async(std::launch::async,
                             [ work = std::move(f) ] { work(); });
#endif
  }


  /// Wait for the execution of the callable on this tile
  void wait() {
    future_work.get();
  }


  /// Configure a connection of the shim AXI stream switch
  void connect(typename geo::core_axi_stream_switch::slave_port_layout sp,
               typename geo::core_axi_stream_switch::master_port_layout mp) {
    axi_ss.connect(sp, mp);
  }


  /// Compute the size of the graphics representation of the processor
  static vec<int, 2> display_core_size() {
    // This is the minimum rectangle fitting all the processor outputs & inputs
    return { 1 + ranges::distance(axi_ss_geo::m_me_range),
             1 + ranges::distance(axi_ss_geo::s_me_range) };
  }


  /// Compute the size of the graphics representation of the tile
  static vec<int, 2> display_size() {
    // Just the sum of the size of its content
    return display_core_size() + axi_ss_t::display_size();
  }


  /// Display the tile to a LaTeX context
  void display(latex::context& c) const {
    auto get_tikz_coordinate = [&] (auto x, auto y) {
      auto const [x_size, y_size] = display_size();
      return (boost::format { "(%1%,%2%)" }
              // Scale real LaTeX coordinate to fit
              % c.scale(x_coordinate*x_size + x)
              % c.scale(y_coordinate*y_size + y)).str();
    };
    c.add((boost::format { "  \\begin{scope}[name prefix = TileX%1%Y%2%]" }
           % x_coordinate % y_coordinate).str());
    axi_ss.display(c, display_core_size(), get_tikz_coordinate);

    // Connect the core receivers to its AXI stream switch
    for (auto [i, p] : axi_ss_geo::m_me_range | ranges::views::enumerate) {
      c.add((boost::format { R"(
    \coordinate(CoreIn%1%) at %2%;
    \node[rotate=90,anchor=east](CoreIn%1%Label) at %2% {in(%1%)};
    \draw (node cs:name=MMe%1%)
       -| (node cs:name=CoreIn%1%);)" }
        % i % get_tikz_coordinate(i,
                                  ranges::distance(axi_ss_geo::m_me_range) + 1)
        ).str());
    };
    // Connect the core senders to its AXI stream switch
    for (auto [i, p] : axi_ss_geo::s_me_range | ranges::views::enumerate) {
      c.add((boost::format { R"(
    \coordinate(CoreOut%1%) at %2%;
    \node[anchor=east](CoreOut%1%Label) at %2%  {out(%1%)};
    \draw (node cs:name=CoreOut%1%)
       -| (node cs:name=SMe%1%);)" }
          % i % get_tikz_coordinate(ranges::distance(axi_ss_geo::s_me_range),
                                    i + 1)).str());
    };
    c.add((boost::format { R"(
    \node() at %1% {\texttt{tile<%2%,%3%>}};
    \begin{scope}[on background layer]
      \node [fill=orange!30, fit={(node cs:name=CoreIn0Label)
                                  (node cs:name=CoreOut0Label)}]
            (Core) {};
    \end{scope}
  \end{scope}

)" } % get_tikz_coordinate(1, 0) % x_coordinate % y_coordinate).str());
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_HPP
