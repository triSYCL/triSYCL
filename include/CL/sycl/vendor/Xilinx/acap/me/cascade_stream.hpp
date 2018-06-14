#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_CASCADE_STREAM_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_CASCADE_STREAM_HPP

/** \file The cascade stream infrastructure between MathEngine tiles

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

namespace cl::sycl::vendor::xilinx::acap::me {

/** The cascade stream infrastructure between MathEngine tiles

    Based on Math Engine (ME) Architecture Specification, Revision v1.4
    March 2018

    3.5.4 Data Movement using Cascade Streams, p. 86

    2.13 Device-level Floorplanning Guidelines, 2.13.2 Array Edges, p. 61

    4.4 ME Core Interfaces, 4.4.4 Cascade Stream Interface, p. 113

    Direct stream interface: One cascade stream in, one cascade stream
    out (384-bits)

*/
struct cascade_stream {
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_CASCADE_STREAM_HPP
