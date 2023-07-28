#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_GEOGRAPHY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_GEOGRAPHY_HPP

/** \file

    Geographic information about the AI Engine CGRA

    Define in a central location most of the ACAP architectural
    parameters.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/format.hpp>
#include <boost/hana.hpp>

#include "triSYCL/detail/ranges.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

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


  /** Compute the linearized coordinate of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  static auto constexpr linear_id(int x, int y) {
    return x + x_size*y;
  }


  /** Compute the horizontal coordinate of a tile from linear id

      \param[in] linear_id is a linearized coordinate
  */
  static auto constexpr linear_x(int linear_id) {
    return linear_id%x_size;
  }


  /** Compute the vertical coordinate of a tile from a linear id

      \param[in] linear_id is a linearized coordinate
  */
  static auto constexpr linear_y(int linear_id) {
    return linear_id/x_size;
  }


  /** Test if an horizontal coordinate is valid

      \param[in] x is the horizontal tile coordinate
  */
  static bool constexpr is_x_valid(int x) {
    return x_min <= x && x <= layout::x_max;
  }


  /** Validate an horizontal coordinate

      Do nothing but throwing if the coordinate is not a valid tile
      coordinate.

      \param[in] x is the horizontal tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  static void validate_x(int x) {
    if (!is_x_valid(x)) {
      throw ::trisycl::runtime_error{""};
    }
  }


  /** Test if a vertical coordinate is valid

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_y_valid(int y) {
    return y_min <= y && y <= layout::y_max;
  }


  /** Validate a vertical coordinate

      Do nothing but throwing if the coordinate is not a valid tile
      coordinate.

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  static void validate_y(int y) {
    if (!is_y_valid(y)) {
      throw ::trisycl::runtime_error{""};
    }
  }


  /** Test if horizontal and vertical coordinates are valid

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_x_y_valid(int x, int y) {
    return is_x_valid(x) && is_y_valid(y);
  }

  /** Validate the horizontal and vertical coordinates

      Do nothing but throwing if the coordinates are not valid tile
      coordinates.

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  static void validate_x_y(int x, int y) {
    validate_x(x);
    validate_y(y);
  }


  /** Test if a tile is in the Western column

      \param[in] x is the horizontal tile coordinate
  */
  static bool constexpr is_west_column(int x) {
    return x == x_min;
  }


  /** Test if a tile is in the Eastern column

      \param[in] x is the horizontal tile coordinate
  */
  static bool constexpr is_east_column(int x) {
    return x == layout::x_max;
  }


  /** Test if a tile is in the Southern row

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_south_row(int y) {
    return y == y_min;
  }


  /** Test if a tile is in the Northern row

      \param[in] y is the vertical tile coordinate
  */
  static bool constexpr is_north_row(int y) {
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
    // FIXME: what to do when the requested tiles do not cover all columns of the hardware.
    return x == ((y & 1) ? x_min : layout::x_max) && y == layout::y_max;
  }


  /** Compute the linearized coordinate of a tile along the cascade
      stream

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate
  */
  static auto constexpr cascade_linear_id(int x, int y) {
    return x_size*y + ((y & 1) ? layout::x_max - x : x);
  }


  /** Compute the horizontal coordinate of a tile from a cascade
      linear id

      \param[in] linear_id is a linear id along the cascade stream
  */
  static auto constexpr cascade_linear_x(int linear_id) {
    auto x = linear_x(linear_id);
    // According to the row parity, symmetrize the x coordinate
    return (cascade_linear_y(linear_id) & 1) ? layout::x_max - x : x;
  }


  /** Compute the vertical coordinate of a tile from a cascade
      linear id

      \param[in] linear_id is a linear id along the cascade stream
  */
  static auto constexpr cascade_linear_y(int linear_id) {
    // Same as the linear id, actually
    return linear_y(linear_id);
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
      // No memory module on the South of the first (Southern) line
      return false;
    else if (y == layout::y_max && dy == 1)
      // No memory module on the North of the last (Northern) line
      return false;
    else if (x == x_min && !(y & 1) && (dx == -1))
      // No memory module on the West of the first (Western) column on
      // even lines
      return false;
    else if (x == layout::x_max && (y & 1) && (dx == 1))
      // No memory module on the East of the last (Eastern) column on
      // odd lines
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


  // The organization of the AXI stream switch on a core tile
  struct core_axi_stream_switch {
    /// Default number of registers on default router paths
    static auto constexpr latency = 4;

    /// Depth of additional FIFO on specific FIFO paths
    static auto constexpr fifo_depth = 16;

    /** Layout of the AXI stream master ports in the switch

        Revision v2.02, 4.2.1 Types of AXI-Streams, Table 4-2, p. 101 */
    enum class master_port_layout : std::int8_t {
      me_0,
      me_1, me_last = me_1, ///< Used for user port validation
      dma_0,
      dma_1, dma_last = dma_1, ///< Used for DMA validation
      tile_ctrl, tile_ctrl_last = tile_ctrl, ///< Used for validation
      fifo_0,
      fifo_1, fifo_last = fifo_1, ///< Used for FIFO validation
      south_0,
      south_1,
      south_2,
      south_3, south_last = south_3, ///< Used for south validation
      west_0,
      west_1,
      west_2,
      west_3, west_last = west_3, ///< Used for west validation
      north_0,
      north_1,
      north_2,
      north_3,
      north_4,
      north_5, north_last = north_5, ///< Used for north validation
      east_0,
      east_1,
      east_2,
      east_3, east_last = east_3, ///< Used for east validation
      // To measure the enum
      size
    };

  private:

    /// Use a hidden friend to have introspection for master_port_layout
    friend bool constexpr is_axi_master(master_port_layout) {
      // Yes, master_port_layout is definitely an AXI master
      return true;
    }

  public:

    /// \todo factorize with a mix-in the following between master and slave

    /// Number of AXI stream master ports in the switch
    static constexpr auto nb_master_port =
      detail::underlying_value(master_port_layout::size);

    /// A range of the master ports connected to the tile inputs
    static auto inline m_me_range =
      views::enum_type(master_port_layout::me_0, master_port_layout::me_last);

    /// Number of master ports connected to the tile inputs
    static constexpr auto m_me_size =
      detail::enum_count(master_port_layout::me_0, master_port_layout::me_last);

    /// A range of the master ports connected to the DMA inputs
    static auto inline m_dma_range =
      views::enum_type(master_port_layout::dma_0, master_port_layout::dma_last);

    /// Number of master ports connected to the DMA inputs
    static constexpr auto m_dma_size =
      detail::enum_count(master_port_layout::dma_0,
                         master_port_layout::dma_last);

    /// A range of the master ports connected to the tile control inputs
    static auto inline m_tile_ctrl_range =
      views::enum_type(master_port_layout::tile_ctrl,
                       master_port_layout::tile_ctrl_last);

    /// Number of master ports connected to the tile control inputs
    static constexpr auto m_tile_ctrl_size =
      detail::enum_count(master_port_layout::tile_ctrl,
                         master_port_layout::tile_ctrl_last);

    /// A range of the master ports connected to the FIFO inputs
    static auto inline m_fifo_range =
      views::enum_type(master_port_layout::fifo_0,
                       master_port_layout::fifo_last);

    /// Number of master ports connected to the FIFO inputs
    static constexpr auto m_fifo_size =
      detail::enum_count(master_port_layout::fifo_0,
                         master_port_layout::fifo_last);

    /// A range of the South master ports
    static auto inline m_south_range =
      views::enum_type(master_port_layout::south_0,
                       master_port_layout::south_last);

    /// Number of South master ports
    static constexpr auto m_south_size =
      detail::enum_count(master_port_layout::south_0,
                         master_port_layout::south_last);

    /// A range of the West master ports
    static auto inline m_west_range =
      views::enum_type(master_port_layout::west_0,
                       master_port_layout::west_last);

    /// Number of West master ports
    static constexpr auto m_west_size =
      detail::enum_count(master_port_layout::west_0,
                         master_port_layout::west_last);

    /// A range of the North master ports
    static auto inline m_north_range =
      views::enum_type(master_port_layout::north_0,
                       master_port_layout::north_last);

    /// Number of North master ports
    static constexpr auto m_north_size =
      detail::enum_count(master_port_layout::north_0,
                         master_port_layout::north_last);

    /// A range of the East master ports
    static auto inline m_east_range =
      views::enum_type(master_port_layout::east_0,
                       master_port_layout::east_last);

    /// Number of East master ports
    static constexpr auto m_east_size =
      detail::enum_count(master_port_layout::east_0,
                         master_port_layout::east_last);

    /** Layout of the AXI stream slave ports in the switch

        Revision v2.02, 4.2.1 Types of AXI-Streams, Table 4-3, p. 102 */
    enum class slave_port_layout : std::int8_t {
      me_0,
      me_1, me_last = me_1, ///< Used for user port validation
      dma_0,
      dma_1, dma_last = dma_1, ///< Used for DMA validation
      tile_ctrl, tile_ctrl_last = tile_ctrl, ///< Used for validation,
      fifo_0,
      fifo_1, fifo_last = fifo_1, ///< Used for FIFO validation
      south_0,
      south_1,
      south_2,
      south_3,
      south_4,
      south_5, south_last = south_5, ///< Used for south validation
      west_0,
      west_1,
      west_2,
      west_3, west_last = west_3, ///< Used for west validation
      north_0,
      north_1,
      north_2,
      north_3, north_last = north_3, ///< Used for north validation
      east_0,
      east_1,
      east_2,
      east_3, east_last = east_3, ///< Used for east validation
      core_trace,
      mem_trace,
      // To measure the enum
      size
    };

  private:

    /// Use a hidden friend to have introspection for slave_port_layout
    friend bool constexpr is_axi_master(slave_port_layout) {
       // No, slave_port_layout is definitely not an AXI master
      return false;
    }

  public:

    /// Number of AXI stream slave ports in the switch
    static constexpr auto nb_slave_port =
      detail::underlying_value(slave_port_layout::size);

    /// A range of the slave ports connected to the tile outputs
    static auto inline s_me_range =
      views::enum_type(slave_port_layout::me_0, slave_port_layout::me_last);

    /// Number of slave ports connected to the tile outputs
    static constexpr auto s_me_size =
      detail::enum_count(slave_port_layout::me_0, slave_port_layout::me_last);

    /// A range of the slave ports connected to the DMA outputs
    static auto inline s_dma_range =
      views::enum_type(slave_port_layout::dma_0, slave_port_layout::dma_last);

    /// Number of slave ports connected to the DMA outputs
    static constexpr auto s_dma_size =
      detail::enum_count(slave_port_layout::dma_0, slave_port_layout::dma_last);

    /// A range of the slave ports connected to the tile control outputs
    static auto inline s_tile_ctrl_range =
      views::enum_type(slave_port_layout::tile_ctrl,
                       slave_port_layout::tile_ctrl_last);

    /// Number of slave ports connected to the tile control outputs
    static constexpr auto s_tile_ctrl_size =
      detail::enum_count(slave_port_layout::tile_ctrl,
                         slave_port_layout::tile_ctrl_last);

    /// A range of the slave ports connected to the FIFO outputs
    static auto inline s_fifo_range =
      views::enum_type(slave_port_layout::fifo_0, slave_port_layout::fifo_last);

    /// Number of slave ports connected to the FIFO outputs
    static constexpr auto s_fifo_size =
      detail::enum_count(slave_port_layout::fifo_0,
                         slave_port_layout::fifo_last);

    /// A range of the South slave ports
    static auto inline s_south_range =
      views::enum_type(slave_port_layout::south_0,
                       slave_port_layout::south_last);

    /// Number of South slave ports
    static constexpr auto s_south_size =
      detail::enum_count(slave_port_layout::south_0,
                         slave_port_layout::south_last);

    /// A range of the West slave ports
    static auto inline s_west_range =
      views::enum_type(slave_port_layout::west_0, slave_port_layout::west_last);

    /// Number of West slave ports
    static constexpr auto s_west_size =
      detail::enum_count(slave_port_layout::west_0,
                         slave_port_layout::west_last);

    /// A range of the North slave ports
    static auto inline s_north_range =
      views::enum_type(slave_port_layout::north_0,
                       slave_port_layout::north_last);

    /// Number of North slave ports
    static constexpr auto s_north_size =
      detail::enum_count(slave_port_layout::north_0,
                         slave_port_layout::north_last);

    /// A range of the East slave ports
    static auto inline s_east_range =
      views::enum_type(slave_port_layout::east_0, slave_port_layout::east_last);

    /// Number of East slave ports
    static constexpr auto s_east_size =
      detail::enum_count(slave_port_layout::east_0,
                         slave_port_layout::east_last);

    /// Describe the AXI stream switch interconnection neighborhood
    static auto inline interconnect = boost::hana::make_tuple(
        // Connection topology of the NoC towards East of the switches
        std::tuple { 1, 0, m_east_range, s_west_range }
        // Connection topology of the NoC towards West of the switches
      , std::tuple { -1, 0, m_west_range, s_east_range }
        // Connection topology of the NoC towards North of the switches
      , std::tuple { 0, 1, m_north_range, s_south_range }
        // Connection topology of the NoC towards South of the switches
      , std::tuple { 0, -1, m_south_range, s_north_range }
      );


  };

  // The organization of the AXI stream switch on a shim tile
  struct shim_axi_stream_switch {
    /// \todo factorize the 2 following values with core tiles?
    /// Default number of registers on default router paths
    static auto constexpr latency = 4;

    /// Depth of additional FIFO on specific FIFO paths
    static auto constexpr fifo_depth = 16;

    /** Layout of the AXI stream master ports in the switch

        Revision v2.02, 6.5.3 Shim AXI-Stream Interconnect, Table 6-2, p. 291 */
    enum class master_port_layout : std::int8_t {
      tile_ctrl, tile_ctrl_last = tile_ctrl, ///< Used for validation,
      fifo_0,
      fifo_1, fifo_last = fifo_1, ///< Used for FIFO validation
      south_0,
      south_1,
      south_2,
      south_3,
      south_4,
      south_5, south_last = south_5, ///< Used for shim BLI port validation
      west_0,
      west_1,
      west_2,
      west_3, west_last = west_3, ///< Used for west validation
      north_0,
      north_1,
      north_2,
      north_3,
      north_4,
      north_5, north_last = north_5, ///< Used for north validation
      east_0,
      east_1,
      east_2,
      east_3, east_last = east_3, ///< Used for east validation
      // To measure the enum
      size
    };

  private:

    /// \todo Factorize out
    /// Use a hidden friend to have introspection for master_port_layout
    friend bool constexpr is_axi_master(master_port_layout) {
      // Yes, master_port_layout is definitely an AXI master
      return true;
    }

  public:

    /// \todo factorize with a mix-in the following between master and slave

    /// Number of AXI stream master ports in the switch
    static constexpr auto nb_master_port =
      detail::underlying_value(master_port_layout::size);

    /// \todo Factorize out this range code with core tile

    /// A range of the master ports connected to the tile control inputs
    static auto inline m_tile_ctrl_range =
      views::enum_type(master_port_layout::tile_ctrl,
                       master_port_layout::tile_ctrl_last);

    /// Number of master ports connected to the tile control inputs
    static constexpr auto m_tile_ctrl_size =
      detail::enum_count(master_port_layout::tile_ctrl,
                         master_port_layout::tile_ctrl_last);

    /// A range of the master ports connected to the FIFO inputs
    static auto inline m_fifo_range =
      views::enum_type(master_port_layout::fifo_0,
                       master_port_layout::fifo_last);

    /// Number of master ports connected to the FIFO inputs
    static constexpr auto m_fifo_size =
      detail::enum_count(master_port_layout::fifo_0,
                         master_port_layout::fifo_last);

    /// A range of the South master ports
    static auto inline m_south_range =
      views::enum_type(master_port_layout::south_0,
                       master_port_layout::south_last);

    /// Number of South master ports
    static constexpr auto m_south_size =
      detail::enum_count(master_port_layout::south_0,
                         master_port_layout::south_last);

    /// A range of the West master ports
    static auto inline m_west_range =
      views::enum_type(master_port_layout::west_0,
                       master_port_layout::west_last);

    /// Number of West master ports
    static constexpr auto m_west_size =
      detail::enum_count(master_port_layout::west_0,
                         master_port_layout::west_last);

    /// A range of the North master ports
    static auto inline m_north_range =
      views::enum_type(master_port_layout::north_0,
                       master_port_layout::north_last);

    /// Number of North master ports
    static constexpr auto m_north_size =
      detail::enum_count(master_port_layout::north_0,
                         master_port_layout::north_last);

    /// A range of the East master ports
    static auto inline m_east_range =
      views::enum_type(master_port_layout::east_0,
                       master_port_layout::east_last);

    /// Number of East master ports
    static constexpr auto m_east_size =
      detail::enum_count(master_port_layout::east_0,
                         master_port_layout::east_last);

    /** Layout of the AXI stream slave ports in the switch

        Revision v2.02, 6.5.3 Shim AXI-Stream Interconnect, Table 6-3, p. 291 */
    enum class slave_port_layout : std::int8_t {
      tile_ctrl, tile_ctrl_last = tile_ctrl, ///< Used for validation,
      fifo_0,
      fifo_1, fifo_last = fifo_1, ///< Used for FIFO validation
      south_0,
      south_1,
      south_2,
      south_3,
      south_4,
      south_5,
      south_6,
      south_7, south_last = south_7, ///< Used for shim BLI port validation
      west_0,
      west_1,
      west_2,
      west_3, west_last = west_3, ///< Used for west validation
      north_0,
      north_1,
      north_2,
      north_3, north_last = north_3, ///< Used for north validation
      east_0,
      east_1,
      east_2,
      east_3, east_last = east_3, ///< Used for east validation
      shim_trace,
      // To measure the enum
      size
    };

  private:

    /// Use a hidden friend to have introspection for slave_port_layout
    friend bool constexpr is_axi_master(slave_port_layout) {
       // No, slave_port_layout is definitely not an AXI master
      return false;
    }

  public:

    /// Number of master AXI stream slave ports in the switch
    static constexpr auto nb_slave_port =
      static_cast<int>(slave_port_layout::size);

    /// A range of the slave ports connected to the tile control outputs
    static auto inline s_tile_ctrl_range =
      views::enum_type(slave_port_layout::tile_ctrl,
                       slave_port_layout::tile_ctrl_last);

    /// Number of slave ports connected to the tile control outputs
    static constexpr auto s_tile_ctrl_size =
      detail::enum_count(slave_port_layout::tile_ctrl,
                         slave_port_layout::tile_ctrl_last);

    /// A range of the slave ports connected to the FIFO outputs
    static auto inline s_fifo_range =
      views::enum_type(slave_port_layout::fifo_0, slave_port_layout::fifo_last);

    /// Number of slave ports connected to the FIFO outputs
    static constexpr auto s_fifo_size =
      detail::enum_count(slave_port_layout::fifo_0,
                         slave_port_layout::fifo_last);

    /// A range of the South slave ports
    static auto inline s_south_range =
      views::enum_type(slave_port_layout::south_0,
                       slave_port_layout::south_last);

    /// Number of South slave ports
    static constexpr auto s_south_size =
      detail::enum_count(slave_port_layout::south_0,
                         slave_port_layout::south_last);

    /// A range of the West slave ports
    static auto inline s_west_range =
      views::enum_type(slave_port_layout::west_0, slave_port_layout::west_last);

    /// Number of West slave ports
    static constexpr auto s_west_size =
      detail::enum_count(slave_port_layout::west_0,
                         slave_port_layout::west_last);

    /// A range of the North slave ports
    static auto inline s_north_range =
      views::enum_type(slave_port_layout::north_0,
                       slave_port_layout::north_last);

    /// Number of North slave ports
    static constexpr auto s_north_size =
      detail::enum_count(slave_port_layout::north_0,
                         slave_port_layout::north_last);

    /// A range of the East slave ports
    static auto inline s_east_range =
      views::enum_type(slave_port_layout::east_0, slave_port_layout::east_last);

    /// Number of East slave ports
    static constexpr auto s_east_size =
      detail::enum_count(slave_port_layout::east_0,
                         slave_port_layout::east_last);

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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_GEOGRAPHY_HPP
