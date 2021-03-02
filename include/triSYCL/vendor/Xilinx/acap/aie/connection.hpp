#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP

/** \file Simple connections between AIE AXI stream switch and inside
    AXI stream switch

    \todo Deprecate \c connection since now we have a more precise routing
    involving the AXI stream switches so we should instead use the
    same data types.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/type_index.hpp>

#include "triSYCL/access.hpp"

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


/** Abstract interface for a communication port

    For example a router output is actually implemented as an input
    to some other consumer (router, core...).
*/
class communicator_port : detail::debug<communicator_port> {

public:

  /// Enqueue a packet on the communicator input
  void virtual write(const axi_packet&) = 0;


  /// Alias to enqueue a packet on the communicator input
  auto& operator<<(const axi_packet::value_type& v) {
    write(v);
    return *this;
  }


  /** Try to enqueue a packet to input

      \return true if the packet is correctly enqueued
  */
  bool virtual try_write(const axi_packet&) = 0;


  /// Waiting read to a core input port
  axi_packet::value_type virtual read() = 0;


  /** Non-blocking read from a communicator output port

      \return true if the value was correctly read
  */
  bool virtual try_read(axi_packet::value_type&) = 0;


  /// Alias to the waiting read to a communicator output port
  template <typename T>
  auto& operator>>(T& v) {
    v = static_cast<T>(read());
    return *this;
  }


  /// Nothing specific to do but need a virtual destructor to avoid slicing
  virtual ~communicator_port() = default;
};


/// A FIFO channel connection
/// \todo factorize out router minion
class fifo_channel : public communicator_port {
  /// FIFO capacity queue
  /// \todo check with hardware team for the value
  auto static constexpr capacity = 16;

  /// Payload data type
  using value_type = axi_packet::value_type;

  /// The underlying FIFO
  boost::fibers::buffered_channel<axi_packet> c { capacity };

public:

  /// Enqueue a packet to the FIFO channel
  void write(const axi_packet &v) override {
    c.push(v);
  }


  /** Try to enqueue a packet to the FIFO channel

      \return true if the packet is correctly enqueued
  */
  bool try_write(const axi_packet &v) override {
    return c.try_push(v) == boost::fibers::channel_op_status::success;
  }


  /// Waiting read from the FIFO channel
  value_type read() override {
    return c.value_pop().data;
  }


  /** Non-blocking read from the FIFO channel

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


/// Abstraction of a communication port
struct port {
  /// Abstraction of a communication port inside a shim tile
  struct shim {
    const int x;
    const int port;
  };
/// Abstraction of a communication port inside a tile
  struct tile {
    const int x;
    const int y;
    const int port;
  };
};

/// A connection between some AXI Stream Switch
struct connection {
  /// The concrete communication media with some type erasure
  std::shared_ptr<std::any> p;

  /** Create a connection

      \param[in] pipe is the communication media to be used
  */
  template <typename Pipe>
  connection(Pipe && pipe)
    : p { new std::any { std::forward<Pipe>(pipe) } } {}


  /// Get the input port of this connection
  auto in() {
    return input { *this };
  }


  /// Get the output port of this connection
  auto out() {
    return output { *this };
  }


  /// Base class of connection port
  struct base {
    /// Keep track of the communication media
    std::shared_ptr<std::any> p;


    /// Construct a port from a connection
    base(const connection &c)
      : p { c.p } {}

    base() = default;


  /** Access the type erased pipe as type T

      \param[in] T is the type of the data transmitted by the
      connection

      \throws trisycl::runtime_error if the connection has not the
      correct type
  */
  template <typename T>
    auto pipe_of() {
      try {
        return std::any_cast<::trisycl::sycl_2_2::static_pipe<T, 4>>(*p);
      } catch (std::bad_any_cast &) {
        throw ::trisycl::runtime_error {
          "The current connection is not of type "s
          + boost::typeindex::type_id<T>().pretty_name() };
      }
    }

  };


  /// Abstract an input port
  struct input : base {
    /// Recycle the base constructors
    using base::base;

    /** Get the input port

        \param[in] InputT is the data type to be used in the transfers

        \param[in] Target specifies if the connection is blocking or
        not

        \throws trisycl::runtime_error if the input is not connected
    */
    template <typename InputT, access::target Target>
    auto in() {
      if (!p)
        throw ::trisycl::runtime_error { "This input is not connected" };
      return
        pipe_of<InputT>().template get_access<access::mode::read, Target>();
    }
  };


  /// Abstract an output port
  struct output : base {
    /// Recycle the base constructors
    using base::base;

    /** Get the output port

        \param[in] InputT is the data type to be used in the transfers

        \param[in] Target specifies if the connection is blocking or
        not

        \throws trisycl::runtime_error if the output is not connected
    */
    template <typename OutputT, access::target Target>
    auto out() {
      if (!p)
        throw ::trisycl::runtime_error { "This output is not connected" };
      return
        pipe_of<OutputT>().template get_access<access::mode::write, Target>();
    }
  };
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP
