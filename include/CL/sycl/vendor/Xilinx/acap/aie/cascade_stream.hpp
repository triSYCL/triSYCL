#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_CASCADE_STREAM_HPP

/** \file

    The cascade stream infrastructure between AI Engine tiles

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/access.hpp"
#include "geography.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** Implementation of the cascade stream infrastructure between AI
    Engine tiles

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    3.5.4 Data Movement using Cascade Streams, p. 86

    2.13 Device-level Floorplanning Guidelines, 2.13.2 Array Edges, p. 61

    4.4 ME Core Interfaces, 4.4.4 Cascade Stream Interface, p. 113

    Direct stream interface: One cascade stream in, one cascade stream
    out (384-bits)
*/
template <typename Geography>
struct cascade_stream {
  using geo = Geography;
  /** The pipes for the cascade streams, with 1 spare pipe on each
      side of PE strings

      \todo Use a data type with 384 bits

      There are 4 registers along the data path according to 1.4
      specification. */
  cl::sycl::static_pipe<int, 4>
  cascade_stream_pipes[geo::x_size*geo::y_size + 1];

  /* Cascade stream layout

      On even rows, a tile use cascade_stream_pipes[y][x] as input and
      cascade_stream_pipes[y][x + 1] as output

      On odd rows the flow goes into the other direction, so a tile
      use cascade_stream_pipes[y][x + 1] as input and
      cascade_stream_pipes[y][x] as output
  */

  /** Get a read accessor to the cascade stream input

      \param T is the data type used to read from the cascade
      stream pipe

      \param Target is the access mode to the pipe. It is blocking
      by default

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  template <typename T, access::target Target>
  auto get_cascade_stream_in(int x, int y) const {
    // On odd rows, the cascade streams goes in the other direction
    return cascade_stream_pipes[geo::x_size*y
                                + ((y & 1) ? geo::x_max - x : x)]
      .template get_access<access::mode::read, Target>();
  }


  /** Get a write accessor to the cascade stream output

      \param T is the data type used to write to the cascade
      stream pipe

      \param Target is the access mode to the pipe. It is blocking
      by default

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  template <typename T, access::target Target>
  auto get_cascade_stream_out(int x, int y) const {
    // On odd rows, the cascade streams goes in the other direction
    return cascade_stream_pipes[geo::x_size*y + 1
                                + ((y & 1) ? geo::x_max - x : x)]
      .template get_access<access::mode::write, Target>();
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
