#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP

/** \file The basic MathEngine tile

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

#include "CL/sycl/access.hpp"

namespace cl::sycl::vendor::xilinx::acap::me {

/** The MathEngine tile infrastructure
 */
template <typename ME_Array, int X, int Y>
struct tile {
  /// The tile coordinates in the grid
  static auto constexpr x = X;
  static auto constexpr y = Y;

  using geo = typename ME_Array::geo;

  /// The thread used to run this tile
  std::thread thread;

  /// Keep a reference to the array owning this tile
  ME_Array *me_array;

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


  void set_array(ME_Array *array) {
    me_array = array;
  }


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

  /// Test if a memory module exists and is connected to this tile
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


  /// Compute the linear id of a memory module attached to this tile
  static auto constexpr memory_module_linear_id(int dx, int dy) {
    return geo:: memory_module_linear_id(x, y, dx, dy);
  }


  /// Get the memory module on the left if it does exist
   auto &mem_left() {
     static_assert(is_memory_module_left(), "There is no memory module"
                   " on the left of this tile in the left column and"
                   " on an even row");
     return me_array->template
      get_memory_module<memory_module_linear_id(-1, 0)>();
  }


  /// Get the memory module on the right if it does exist
  auto &mem_right() {
    static_assert(is_memory_module_right(), "There is no memory module"
                  " on the right of this tile in the right column and"
                   " on an odd row");
    return me_array->template
      get_memory_module<memory_module_linear_id(1, 0)>();
  }


  /// Get the memory module below if it does exist
  auto &mem_down() {
    static_assert(is_memory_module_down(), "There is no memory module"
                  " below the lower tile row");
    return me_array->template
      get_memory_module<memory_module_linear_id(0, -1)>();
  }


  /// Get the memory module above if it does exist
  auto &mem_up() {
    static_assert(is_memory_module_up(), "There is no memory module"
                  " above the upper tile row");
    return me_array->template
      get_memory_module<memory_module_linear_id(0, 1)>();
  }


  /// The memory module native to the tile
  auto &mem() {
    if constexpr (y & 1)
      return mem_left();
    else
      return mem_right();
  }


  /* Provide a run member function that does nothing so it is possible
     to write a minimum MathEngin program that does nothing.

     Note that this function is not virtual but the common case is
     that a programmer can implement it to specify the program done by
     a tile
   */
  void run() {
  }


  /// Test if this tile owns the start of the cascade_stream
  static bool constexpr is_cascade_start() {
    return geo::is_cascade_start(x, y);
  }


  /// Test if this tile owns the end of the cascade_stream
  static bool constexpr is_cascade_end() {
    return geo::is_cascade_end(x, y);
  }


  template <typename T, access::target Target = access::target::blocking_pipe>
  auto get_cascade_stream_in() {
    static_assert(!is_cascade_start(), "You cannot access to the cascade stream"
                  " input on the tile that starts the stream");
    return me_array->cs.template get_cascade_stream_in<T, Target>(x, y);
  }


  template <typename T, access::target Target = access::target::blocking_pipe>
  auto get_cascade_stream_out() {
    static_assert(!is_cascade_end(), "You cannot access to the cascade stream"
                  " output on the tile that starts the stream");
    return me_array->cs.template get_cascade_stream_out<T, Target>(x, y);
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
