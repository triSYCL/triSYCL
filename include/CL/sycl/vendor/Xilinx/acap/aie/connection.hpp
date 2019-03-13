#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_ARRAY_CONNECTION_HPP

/** \file

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/access.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{


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
  };


  /// Abstract an input port
  struct input : base {
    /// Recycle the base constructors
    using base::base;

    /** Get the input port

        \param[in] InputT is the data type to be used in the transfers

        \param[in] Target specifies if the connection is blocking or
        not
    */
    template <typename InputT, access::target Target>
    auto in() {
      if (!p)
        throw "This input has not been connected";
      return std::any_cast<cl::sycl::static_pipe<InputT, 4>>(*p)
        .template get_access<access::mode::read, Target>();
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
    */
    template <typename OutputT, access::target Target>
    auto out() {
      if (!p)
        throw "This output has not been connected";
      return std::any_cast<cl::sycl::static_pipe<OutputT, 4>>(*p)
        .template get_access<access::mode::write, Target>();
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
