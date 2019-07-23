#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP

/** \file

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/type_index.hpp>

#include "triSYCL/access.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

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
      right type
  */
  template <typename T>
    auto pipe_of() {
      try {
        return std::any_cast<::trisycl::static_pipe<T, 4>>(*p);
      } catch (std::bad_any_cast) {
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
