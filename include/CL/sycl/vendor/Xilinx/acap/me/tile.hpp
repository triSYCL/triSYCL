#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP

/** \file The basic MathEngine tile

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include <thread>

namespace cl::sycl::vendor::xilinx::acap::me {

/** The MathEngine tile infrastructure
 */
template <typename Geography, typename ME_Array, int X, int Y>
struct tile {
  /// The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  using geo = Geography;

  /// Keep a reference to the array owning this tile
  ME_Array *me_array;

  /// The thread used to run this tile
  std::thread thread;

  static bool constexpr is_noc() {
    return geo::is_noc_tile(x, y);
  }

  static bool constexpr is_pl() {
    return geo::is_pl_tile(x, y);
  }

  static bool constexpr is_shim() {
    return geo::is_shim_tile(x, y);
  }

  template <int Dim>
  static auto constexpr get_id() {
    static_assert(0 <= Dim && Dim <= 1,
                  "The dimension has to be between 0 and 1");
    if constexpr (Dim == 0)
      return x;
    else
      return y;
  }


  void set_array(ME_Array *array) {
    me_array = array;
  }
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP
