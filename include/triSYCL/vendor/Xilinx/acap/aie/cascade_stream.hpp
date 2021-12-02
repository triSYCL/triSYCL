#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP

#ifdef __SYCL_XILINX_AIE__
#error "This should only exist in emulation mode"
#endif

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
  detail::sycl_2_2::pipe<std::array<char, 48>> stream;

  cascade_stream() : stream{4} {}

  void write48(const char* ptr) {
    static_assert(sizeof(std::array<char, 48>) == 48, "");
    std::array<char, 48> data;
    std::memcpy(data.data(), ptr, 48);
    stream.write(data, /*blocking*/ true);
  }
  void read48(char* ptr) {
    static_assert(sizeof(std::array<char, 48>) == 48, "");
    std::array<char, 48> data;
    stream.read(data, /*blocking*/ true);
    std::memcpy(ptr, data.data(), 48);
  }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP
