#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP

/** \file

    The cascade stream infrastructure between AI Engine tiles

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/access.hpp"
#include "triSYCL/sycl_2_2/static_pipe.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** Implementation of the cascade stream infrastructure between AI
    Engine tiles

    Based on Math Engine (ME) Architecture Specification, Revision v2.01

    March 2019

    3.5.4 Data Movement using Cascade Streams, p. 93

    2.14 Device-level Floorplanning Guidelines, 2.14.2 Array Edges, p. 69

    4.4 ME Core Interfaces, 4.4.4 Cascade Stream Interface, p. 120
    explains there is a FIFO depth of 4

    5.9.3.6 Cascade Streams: explains that there are only blocking
    accesses

    Direct stream interface: One cascade stream in, one cascade stream
    out (384-bits)
*/

struct cascade_stream {
  ::trisycl::sycl_2_2::static_pipe<int, 4> stream;

  template <typename T> auto get_cascade_stream_in() const {
    return stream.template get_access<access::mode::read,
                                      access::target::blocking_pipe>();
  }

  /** Get a blocking write accessor to the cascade stream output

      \param T is the data type used to write to the cascade
      stream pipe

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  template <typename T> auto get_cascade_stream_out() const {
    // The output is connected to the down-stream neighbour of the cascade
    return stream.template get_access<access::mode::write,
                                      access::target::blocking_pipe>();
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP
