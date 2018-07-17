#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_ME_TILE_HPP

/** \file The basic MathEngine tile

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

#include <type_traits>

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
  template <typename Dependent = tile,
            typename = std::enable_if_t<Dependent::is_memory_module_left()>>
  auto &mem_left() {
    return me_array->template
      get_memory_module<memory_module_linear_id(-1, 0)>();
  }


  /// Get the memory module on the right if it does exist
  template <typename Dependent = tile,
            typename = std::enable_if_t<Dependent::is_memory_module_right()>>
  auto &mem_right() {
    return me_array->template
      get_memory_module<memory_module_linear_id(1, 0)>();
  }


  /// Get the memory module below if it does exist
  template <typename Dependent = tile,
            typename = std::enable_if_t<Dependent::is_memory_module_down()>>
  auto &mem_down() {
    return me_array->template
      get_memory_module<memory_module_linear_id(0, -1)>();
  }


  /// Get the memory module above if it does exist
  template <typename Dependent = tile,
            typename = std::enable_if_t<Dependent::is_memory_module_up()>>
  auto &mem_up() {
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


  /// Test if this tile owns the start of the cascade_stream
  static bool constexpr is_cascade_start() {
    return geo::is_cascade_start(x, y);
  }


  /// Test if this tile owns the end of the cascade_stream
  static bool constexpr is_cascade_end() {
    return geo::is_cascade_end(x, y);
  }


  template <typename T>
  auto& get_cascade_stream_in() {
    return me_array->get_cascade_stream_in(x, y);
  }


  template <typename T>
  auto get_cascade_stream_out() {
    return me_array->get_cascade_stream_out(x, y);
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
