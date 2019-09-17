#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP

/** \file

    SYCL-friendly device view of AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "geography.hpp"
#include "queue.hpp"
#include "tile_infrastructure.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/
template <typename Layout>
struct device {

  /// The geography of the CGRA
  using geo = geography<Layout>;

  /// The cascade stream infrastructure of the CGRA
  cascade_stream<geo> cs;

  /** Keep track of all the tiles as a type-erased tile_base type to
      have a simpler access to the basic position-independent tile
      features */
  tile_infrastructure<device> tile_bases[geo::y_size][geo::x_size];


  /** Access to the common infrastructure part of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto &tile(int x, int y) {
    geo::validate_x_y(x, y);
    return tile_bases[y][x];
  }


  /** The shim tiles on the lower row of the tile array

      For now we consider only homogeneous shim tiles.
  */
  shim_tile<device> shims[geo::x_size];


  /** Access to the shim tile

      \param[in] x is the horizontal coordinate of the shim tile

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto &shim(int x) {
    geo::validate_x(x);
    return shims[x];
  }


  /// Create a queue on this device
  auto queue() {
    return vendor::xilinx::acap::aie::queue { *this };
  }


  /** Connect the ports of 2 tiles or shims together with a switched
      circuit

      \param[in] T is the type of the data to be transferred

      \param[in] SrcPort is the type of the source port, such as
      port::tile or port::shim

      \param[in] DstPort is the type of the destination port, such as
      port::tile or port::shim

      \throws trisycl::runtime_error if some coordinates or port
      numbers are invalid

      \todo Refactor, make the difference between user & physical ports
  */
  template <typename T, typename SrcPort, typename DstPort>
  void connect(SrcPort src, DstPort dst) {
    /// \todo move this into a factory
    connection c { ::trisycl::static_pipe<T, 4> {} };
    constexpr bool valid_src = std::is_same_v<SrcPort, port::tile>
      || std::is_same_v<SrcPort, port::shim>;
    static_assert(valid_src,
                  "SrcPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<SrcPort, port::tile>) {
       tile(src.x, src.y).out_connection(src.port) = c.out();
    }
    else if constexpr(std::is_same_v<SrcPort, port::shim>) {
       shim(src.x).bli_out_connection(src.port) = c.out();
    }
    constexpr bool valid_dst = std::is_same_v<DstPort, port::tile>
      || std::is_same_v<DstPort, port::shim>;
    static_assert(valid_dst,
                  "DstPort type should be port::tile or port::shim");
    if constexpr (std::is_same_v<DstPort, port::tile>) {
      tile(dst.x, dst.y).in_connection(dst.port) = c.in();
    }
    else if constexpr(std::is_same_v<DstPort, port::shim>) {
      shim(dst.x).bli_in_connection(dst.port) = c.in();
    }
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
