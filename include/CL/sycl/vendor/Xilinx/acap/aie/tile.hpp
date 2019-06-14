#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_HPP

/** \file

    The basic AI Engine heterogeneous tile (i.e. dependent of x & y
    coordinates)

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

#include "CL/sycl/access.hpp"
#include "tile_base.hpp"

namespace cl::sycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure defining the program of a tile

    This is the type you need to inherit from to define the program of
    a CGRA tile.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents

    \param X is the horizontal coordinate of the memory module

    \param Y is the vertical coordinate of the memory module
*/
template <typename AIE, int X, int Y>
struct tile : tile_base<AIE> {
  /** The horizontal tile coordinates in the CGRA grid (starting at 0
      and increasing to the right) */
  static auto constexpr x = X;
  /** The vertical tile coordinates in the CGRA grid (starting at
      increasing to the top) */
  static auto constexpr y = Y;

  /// The geography of the CGRA
  using geo = typename AIE::geo;

  /// Shortcut to the tile base class
  using tb = tile_base<AIE>;

  /** Return the coordinate of the tile in the given dimension

      \param Dim is 0 for the horizontal (x) dimension and 1 for the
      vertical (y) dimension
  */
  template <int Dim>
  static auto constexpr get_id() {
    static_assert(0 <= Dim && Dim <= 1,
                  "The dimension has to be between 0 and 1");
    if constexpr (Dim == 0)
      return x;
    else
      return y;
  }


  /// Return the linearized coordinate of the tile
  static auto constexpr get_linear_id() {
    return geo::linear_id(x, y);
  }

  /** \todo Need something more modern than Boost.Operator to
      implement cl::sycl::id

      Look for example at https://github.com/taocpp/operators/issues/9
  static auto constexpr get_id() {
    return cl::sycl::id<2> { x, y };
  }
  */


  /// Test if the tile is in the left column
  static bool constexpr is_left_column() {
    return geo::is_left_column(x);
  }


  /// Test if the tile is in the right column
  static bool constexpr is_right_column() {
    return geo::is_right_column(x);
  }


  /// Test if the tile is in the bottom row
  static bool constexpr is_bottom_row() {
    return geo::is_bottom_row(y);
  }


  /// Test if the tile is in the top row
  static bool constexpr is_top_row() {
    return geo::is_top_row(y);
  }


  /** Test if a memory module exists and is connected to this tile

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static bool constexpr is_memory_module(int dx, int dy) {
    return geo::is_memory_module(x, y, dx, dy);
  }


  /// Test if a memory module exists on the left of this tile
  static bool constexpr is_memory_module_left() {
    return is_memory_module(-1, 0);
  }


  /// Test if a memory module exists on the right of this tile
  static bool constexpr is_memory_module_right() {
    return is_memory_module(1, 0);
  }


  /// Test if a memory module exists below this tile
  static bool constexpr is_memory_module_down() {
    return is_memory_module(0, -1);
  }


  /// Test if a memory module exists above this tile
  static bool constexpr is_memory_module_up() {
    return is_memory_module(0, 1);
  }


  /** Compute the linear id of a memory module attached to this tile

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static auto constexpr memory_module_linear_id(int dx, int dy) {
    return geo:: memory_module_linear_id(x, y, dx, dy);
  }


  /// Get the memory module on the left if it does exist
   auto &mem_left() {
     static_assert(is_memory_module_left(), "There is no memory module"
                   " on the left of this tile in the left column and"
                   " on an even row");
     return tb::aie_array->template
       memory_module<memory_module_linear_id(-1, 0)>();
  }


  /// Get the memory module on the right if it does exist
  auto &mem_right() {
    static_assert(is_memory_module_right(), "There is no memory module"
                  " on the right of this tile in the right column and"
                   " on an odd row");
    return tb::aie_array->template
      memory_module<memory_module_linear_id(1, 0)>();
  }


  /// Get the memory module below if it does exist
  auto &mem_down() {
    static_assert(is_memory_module_down(), "There is no memory module"
                  " below the lower tile row");
    return tb::aie_array->template
      memory_module<memory_module_linear_id(0, -1)>();
  }


  /// Get the memory module above if it does exist
  auto &mem_up() {
    static_assert(is_memory_module_up(), "There is no memory module"
                  " above the upper tile row");
    return tb::aie_array->template
      memory_module<memory_module_linear_id(0, 1)>();
  }


  /// The memory module native to the tile
  auto &mem() {
    if constexpr (y & 1)
      return mem_left();
    else
      return mem_right();
  }


  /// The type of the memory module native to the tile
  using mem_t = typename AIE::template tileable_memory<x, y>;


  /// Test if this tile owns the start of the cascade_stream
  static bool constexpr is_cascade_start() {
    return geo::is_cascade_start(x, y);
  }


  /// Test if this tile owns the end of the cascade_stream
  static bool constexpr is_cascade_end() {
    return geo::is_cascade_end(x, y);
  }


  /** Compute the linearized coordinate of a tile along the cascade
      stream */
  static auto constexpr cascade_linear_id() {
    return geo::cascade_linear_id(x, y);
  }


  /** Get a read accessor to the cascade stream input

      \param T is the data type used to read from the cascade
      stream pipe

      \param Target is the access mode to the pipe. It is blocking
      by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto get_cascade_stream_in() {
    static_assert(!is_cascade_start(), "You cannot access to the cascade stream"
                  " input on the tile that starts the stream");
    return tb::aie_array->cs.template get_cascade_stream_in<T, Target>(x, y);
  }


  /** Get a write accessor to the cascade stream output

      \param T is the data type used to write to the cascade
      stream pipe

      \param Target is the access mode to the pipe. It is blocking
      by default
  */
  template <typename T, access::target Target = access::target::blocking_pipe>
  auto get_cascade_stream_out() {
    static_assert(!is_cascade_end(), "You cannot access to the cascade stream"
                  " output on the tile that starts the stream");
    return tb::aie_array->cs.template get_cascade_stream_out<T, Target>(x, y);
  }


  /** An horizontal barrier using a lock

      Implement a barrier across the tiles a line.

      \param[in] lock is the ME lock to use. The lock 14 is used by
      default
  */
  void horizontal_barrier(int lock = 14) {
    if constexpr (y & 1) {
      // Propagate a token from left to right and back
      if constexpr (!is_left_column()) {
        // Wait for the left neighbour to be ready
        mem().lu.locks[lock].acquire_with_value(true);
      }
      if constexpr (is_memory_module_right()) {
        mem_right().lu.locks[lock].acquire_with_value(false);
        // Unleash the right neighbour
        mem_right().lu.locks[lock].release_with_value(true);
        // Wait for the right neighbour to acknowledge
        mem_right().lu.locks[lock].acquire_with_value(false);
       }
      if constexpr (!is_left_column()) {
        // Acknowledge to the left neighbour
        mem().lu.locks[lock].release_with_value(false);
      }
    } else {
      // Propagate a token from right to left and back
      if constexpr (!is_right_column()) {
        // Wait for the right neighbour to be ready
        mem().lu.locks[lock].acquire_with_value(true);
      }
      if constexpr (is_memory_module_left()) {
        mem_left().lu.locks[lock].acquire_with_value(false);
        // Unleash the left neighbour
        mem_left().lu.locks[lock].release_with_value(true);
        // Wait for the left neighbour to acknowledge
        mem_left().lu.locks[lock].acquire_with_value(false);
       }
      if constexpr (!is_right_column()) {
        // Acknowledge to the right neighbour
        mem().lu.locks[lock].release_with_value(false);
      }
    }
  }


  /** A vertical barrier using a lock

      Implement a barrier across the tiles a line.

      \param[in] lock is the ME lock to use. The lock 15 is used by
      default
  */
  void vertical_barrier(int lock = 15) {
    // Propagate a token from bottom to top and back
    if constexpr (!is_bottom_row()) {
      // Wait for the neighbour below to be ready
      mem().lu.locks[lock].acquire_with_value(true);
    }
    if constexpr (is_memory_module_up()) {
      mem_up().lu.locks[lock].acquire_with_value(false);
      // Unleash the neighbour above
      mem_up().lu.locks[lock].release_with_value(true);
      // Wait for the neighbour above to acknowledge
      mem_up().lu.locks[lock].acquire_with_value(false);
    }
    if constexpr (!is_bottom_row()) {
      // Acknowledge to the neighbour below
      mem().lu.locks[lock].release_with_value(false);
    }
  }


  /** Full barrier using the 2 locks by default

      Implement a barrier across the full tile array by using \c
      horizontal_barrier() and \c vertical_barrier().
  */
  void barrier() {
    horizontal_barrier();
    vertical_barrier();
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_HPP
