#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_TILE_HPP

/** \file The basic MathEngine tile

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

namespace cl::sycl::vendor::xilinx::acap {

/** The MathEngine tile infrastructure
 */
template <typename Geography, typename ME_Array, int X, int Y>
struct tile {
  // The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  using geography = Geography;

  static bool constexpr is_noc() {
    return geography::is_noc_tile(x, y);
  }

  static bool constexpr is_pl() {
    return geography::is_pl_tile(x, y);
  }

  static bool constexpr is_shim() {
    return geography::is_shim_tile(x, y);
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
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_TILE_HPP
