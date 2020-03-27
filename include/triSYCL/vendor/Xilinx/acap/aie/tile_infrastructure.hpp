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

#include "axi_stream_switch.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/detail/ranges.hpp"

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

  /// The AXI stream switch of the core tile
  axi_ss_t axi_ss;

  /// Keep track of execution in this tile
  std::future<void> work;

private:

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

    /// Enqueue a packet to the core input
    void write(const axi_packet &v) override {
      TRISYCL_DUMP_T("core_receiver " << this << " on fiber "
                     << boost::this_fiber::get_id()
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


    /// Waiting read to a core input port
    value_type read() override {
      TRISYCL_DUMP_T("core_receiver " << this << " on fiber "
                     << boost::this_fiber::get_id()
                     << " read from buffered_channel " << &c);
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


  /// Start the tile infrastructure associated to the AIE device
  void start(detail::fiber_pool &fiber_executor) {
    axi_ss.start(fiber_executor);
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
    /* The input port for the core is actually the corresponding
       output on the switch */
    return *axi_ss.out_connection(translate_output_port(port));
  }


  /** Get the user output port to the AXI stream switch

      \param[in] port is the port to use
  */
  auto& out(int port) {
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
    // Launch the tile program immediately on a new thread
    work = std::async(std::launch::async,
                      std::forward<Work>(f));
  }


  /// Wait for the execution of the callable on this tile
  void wait() {
    work.get();
  }


  /// Configure a connection of the shim AXI stream switch
  void connect(typename geo::core_axi_stream_switch::slave_port_layout sp,
               typename geo::core_axi_stream_switch::master_port_layout mp) {
    axi_ss.connect(sp, mp);
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
