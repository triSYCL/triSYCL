#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_GEOGRAPHY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_GEOGRAPHY_HPP

/** \file

    Geographic information about the AI Engine CGRA

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/hana.hpp>

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** Some geographic information about the AI Engine array

    \param Layout is a class describing the basic layout of the CGRA,
    setting for example the x & y size
*/
template <typename Layout>
struct geography : Layout {
  /// Keep the layout used for this CGRA
  using layout = Layout;

  /// Minimum horizontal coordinate
  static auto constexpr x_min = 0;

  /// Minimum vertical coordinate
  static auto constexpr y_min = 0;

  /// Number of tile horizontally in the CGRA
  static auto constexpr x_size = layout::x_max + 1;

  /// Number of tile vertically in the CGRA
  static auto constexpr y_size = layout::y_max + 1;

  /// Total number of tiles in the CGRA
  static auto constexpr size = x_size*y_size;


  /** Compute the linearized coordinate of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  static auto constexpr linear_id(int x, int y) {
    return x + x_size*y;
  }

  /** A tuple with the coordinate tuples of all the tiles inside the
      CGRA geometry */
  static auto constexpr tile_coordinates = boost::hana::cartesian_product(
    boost::hana::make_tuple(
        boost::hana::range_c<int, y_min, y_size>
      , boost::hana::range_c<int, x_min, x_size>
                            )
                                                                          );


  /** Generate a tuple by tiling the Tileable object onto the CGRA geometry

     \param Tileable is the type to tile according to coordinate \p X
     and \p Y
  */
  template <template <int X, int Y> typename Tileable>
  static auto generate_tiles() {
    return boost::hana::transform(
        tile_coordinates
      , [] (auto coord) {
          return Tileable<boost::hana::at_c<1>(coord),
                          boost::hana::at_c<0>(coord)> {};
        }
                                  );
  }


  /** Test if an horizontal coordinate is valid

      \param[in] x is the horizontal tile coordinate
  */
  static bool constexpr is_x_valid(int x) {
    return x_min <= x && x <= layout::x_max;
  }


  /** Test if a vertical coordinate is valid

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_y_valid(int y) {
    return y_min <= y && y <= layout::y_max;
  }


  /** Test if a tile is in the left column

      \param[in] x is the horizontal tile coordinate
  */
  static bool constexpr is_left_column(int x) {
    return x == x_min;
  }


  /** Test if a tile is in the right column

      \param[in] x is the horizontal tile coordinate
  */
  static bool constexpr is_right_column(int x) {
    return x == layout::x_max;
  }


  /** Test if a tile is in the bottom row

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_bottom_row(int y) {
    return y == y_min;
  }


  /** Test if a tile is in the top row

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_top_row(int y) {
    return y == layout::y_max;
  }


  /** Test if a tile owns the start of the cascade_stream

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_cascade_start(int x, int y) {
    return x == x_min && y == y_min;
  }


  /** Test if a tile owns the end of the cascade_stream

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_cascade_end(int x, int y) {
    // The x position depends on the parity of the last line
    return x == ((y & 1) ? x_min : layout::x_max) && y == layout::y_max;
  }


  /** Test if a memory module coordinate offset is plausible

      1 of the displacement is 0 and the other one is -1 or 1

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static bool constexpr is_valid_memory_module_offset(int dx, int dy) {
    if (dx == 0)
     // Cannot use std::abs() because it is not constexpr in C++17
      return dy == -1 || dy == 1;
    else if (dy == 0)
      return dx == -1 || dx == 1;
    else return false;
  }


  /** Test if a memory module exists and is connected to a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static bool constexpr is_memory_module(int x, int y, int dx, int dy) {
    if (!is_valid_memory_module_offset(dx, dy))
      return false;
    if (y == y_min && dy == -1)
      // No memory module below the first line
      return false;
    else if (y == layout::y_max && dy == 1)
      // No memory module above the last line
      return false;
    else if (x == x_min && !(y & 1) && (dx == -1))
      // No memory module on the left of the first column on even lines
      return false;
    else if (x == layout::x_max && (y & 1) && (dx == 1))
      // No memory module on the right of the last column on odd lines
      return false;
    else return true;
  }


  /** Compute the memory module x coordinate delta from its tile

      \param[in] y is the vertical tile coordinate

      \param[in] dx is the horizontal displacement (-1,0,+1)
  */
  static auto constexpr memory_module_dx_coordinate(int y, int dx) {
    // Mimic Figure 2-20 on p. 65
    return (dx - 1)/2 + (y & 1);
  }


  /** Compute the linear id of a memory module attached to a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static auto constexpr memory_module_linear_id(int x, int y, int dx, int dy) {
    if (dx == 0)
      return linear_id(x, y + dy);
    else
      // Take into account the horizontal chessboard-like skewing
      return linear_id(x + memory_module_dx_coordinate(y, dx), y);
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_GEOGRAPHY_HPP
