#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_HPP

/** \file

    The basic AI Engine heterogeneous tile that dependent of x & y
    coordinates but also from the collective program run on it.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <type_traits>

#include "triSYCL/access.hpp"
#include "triSYCL/detail/program_manager.hpp"
#include "tile_base.hpp"
#include "xaie_wrapper.hpp"
#include "hardware.hpp"
#include "program.hpp"
#include "lock.hpp"


/// TODO: Perhaps worth pushing all Lib X AI Engine functionallity we use down
/// into a C++ API so it can all be excluded with one #ifdef and kept nice and
/// cleanly

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{


/** The AI Engine tile infrastructure defining the program of a tile

    This is the type you need to inherit from to define the program of
    a CGRA tile.

    \param AIE_Program is the type representing the full CGRA with the
    programs and memory contents

    \param X is the horizontal coordinate of the memory module

    \param Y is the vertical coordinate of the memory module

    \todo (Andrew Gozillon) FYI, until Intel fix the way they handle
    base classes in the SYCL compiler frontend this tile_base can
    never have non-default constructed data (on the device side
    implementation, the host should be fine to have whatever). As the
    hack I have in place just now only outlines a call to the base
    classes default constructor, it might be feasible to work around
    that though by calling a base class "fill" function in the tile
    constructor or tweaking the way the run function of a tile is
    outlined so that it outlines it as a copy of the full object
    rather than components of the object and reconstructing it inside
    the kernel (although you'll fall into the standard layout trap as
    you'll be passing a full object to the kernel invocation and it'd
    take some reworking of the Tile's main generation).

    A proper fix for this is probably quite difficult as you'd need to
    take into account the base classes dependencies among-st other
    things like the constructor body not being available in the same
    TU, some discussion here: https://github.com/intel/llvm/issues/488
*/
template <typename AIE_Program, int X, int Y>
struct tile : tile_base<AIE_Program> {
  /** The horizontal tile coordinates in the CGRA grid (starting at 0
      and increasing towards the East) */
  static auto constexpr x = X;
  /** The vertical tile coordinates in the CGRA grid (starting at
      increasing towards the North) */
  static auto constexpr y = Y;

  /// The geography of the CGRA
  using geo = typename AIE_Program::geo;

  /// Shortcut to the tile base class
  using tb = tile_base<AIE_Program>;

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
  static auto constexpr linear_id() {
    return geo::linear_id(x, y);
  }

  /** \todo Need something more modern than Boost.Operator to
      implement trisycl::id

      Look for example at https://github.com/taocpp/operators/issues/9
  static auto constexpr get_id() {
    return trisycl::id<2> { x, y };
  }
  */


  /// Test if the tile is in the Western column
  static bool constexpr is_west_column() {
    return geo::is_west_column(x);
  }


  /// Test if the tile is in the Eastern column
  static bool constexpr is_east_column() {
    return geo::is_east_column(x);
  }


  /// Test if the tile is in the Southern row
  static bool constexpr is_south_row() {
    return geo::is_south_row(y);
  }


  /// Test if the tile is in the Northern row
  static bool constexpr is_north_row() {
    return geo::is_north_row(y);
  }


  /// Test if the tile has a Western neighbor
  static bool constexpr is_west_valid() {
    return !geo::is_west_column(x);
  }


  /// Test if the tile has a Eastern neighbor
  static bool constexpr is_east_valid() {
    return !geo::is_east_column(x);
  }


  /// Test if the tile has a Southern neighbor
  static bool constexpr is_south_valid() {
    return !geo::is_south_row(y);
  }


  /// Test if the tile has a South West neighbor
  static bool constexpr is_south_west_valid() {
    return is_south_valid() && is_west_valid();
  }


  /// Test if the tile has a South East neighbor
  static bool constexpr is_south_east_valid() {
    return is_south_valid() && is_east_valid();
  }


  /// Test if the tile has a Northern neighbor
  static bool constexpr is_north_valid() {
    return !geo::is_north_row(y);
  }


  /// Test if the tile has a North East neighbor
  static bool constexpr is_north_east_valid() {
    return is_north_valid() && is_east_valid();
  }


  /// Test if the tile has a North West neighbor
  static bool constexpr is_north_west_valid() {
    return is_north_valid() && is_west_valid();
  }


  /** Test if a memory module exists and is connected to this tile

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static bool constexpr is_memory_module(int dx, int dy) {
    return geo::is_memory_module(x, y, dx, dy);
  }


  /// Test if a memory module exists on the West of this tile
  static bool constexpr is_memory_module_west() {
    return is_memory_module(-1, 0);
  }


  /// Test if a memory module exists on the East of this tile
  static bool constexpr is_memory_module_east() {
    return is_memory_module(1, 0);
  }


  /// Test if a memory module exists on the South this tile
  static bool constexpr is_memory_module_south() {
    return is_memory_module(0, -1);
  }


  /// Test if a memory module exists on the North this tile
  static bool constexpr is_memory_module_north() {
    return is_memory_module(0, 1);
  }


  /** Compute the linear id of a memory module attached to this tile

      \param[in] dx is the horizontal displacement (-1,0,+1)

      \param[in] dy is the vertical displacement (-1,0,+1)
  */
  static auto constexpr memory_module_linear_id(int dx, int dy) {
    return geo:: memory_module_linear_id(x, y, dx, dy);
  }

  using tile_t = hw_tile<X, Y>;
  using dir = typename tile_t::dir;


#ifdef __SYCL_DEVICE_ONLY__
  __attribute__((noinline)) void log(const char* ptr) {
    hw_mem::log_record* lr = hw_mem::log_record::get(X, Y);
    while (*ptr)
      lr->get_data()[lr->size++] = *(ptr++);
  }
#else
  void log(const char* ptr) {
    std::cout << ptr;
  }
#endif

/// This could be refactored to minimized duplication by separating between host
/// and device at address computation instead of all the function.
#ifndef __SYCL_DEVICE_ONLY__
  /// On the host

  /// Get the memory module on the West if it does exist
   auto &mem_west() {
     static_assert(is_memory_module_west(), "There is no memory module"
                   " on the West of this tile in the Western column and"
                   " on an even row");
     return tb::program->template
       memory_module<memory_module_linear_id(-1, 0)>();
  }


  /// Get the memory module on the East if it does exist
  auto &mem_east() {
    static_assert(is_memory_module_east(), "There is no memory module"
                  " on the East of this tile in the Eastern column and"
                  " on an odd row");
    return tb::program->template
      memory_module<memory_module_linear_id(1, 0)>();
  }


  /// Get the memory module on the South if it does exist
  auto &mem_south() {
    static_assert(is_memory_module_south(), "There is no memory module"
                  " below the Southern tile row");
    return tb::program->template
      memory_module<memory_module_linear_id(0, -1)>();
  }


  /// Get the memory module on the North if it does exist
  auto &mem_north() {
    static_assert(is_memory_module_north(), "There is no memory module"
                  " above the Northern tile row");
    return tb::program->template
      memory_module<memory_module_linear_id(0, 1)>();
  }


  /// The memory module native to the tile
  auto &mem() {
    if constexpr (y & 1)
      return mem_west();
    else
      return mem_east();
  }

#ifdef __SYCL_XILINX_AIE__
/// TODO: This API doesn't yet exist for CPU emulation
  device_lock get_lock(int8_t id, dir d = dir::self) {
    TRISYCL_DUMP2(std::dec << "get_lock: (X = " << X << ", Y = " << Y
                           << ") dir:" << tile_t::dir_to_str[d],
                  "sync");
    return {id, get_dev_handle().inst,
            xaie::XAie_TileLoc(get_dev_handle().tile.Col + tile_t::get_offset(d).y,
                               get_dev_handle().tile.Row + tile_t::get_offset(d).x)};
  }
#endif
#else
  /// On device

  /// The type of the memory module of a tile at offset (dx, dy) from the
  /// current tile.
  template <int dx, int dy>
  using tile_mem_t =
      typename AIE_Program::template tileable_memory<X + dx, Y + dy>;

  /// Get the memory module on the left if it does exist
  auto &mem_west() {
    static_assert(is_memory_module_west(),
                  "There is no memory module"
                  " on the left of this tile in the left column and"
                  " on an even row");
    return *(tile_mem_t<tile_t::get_pos(dir::left).x,
                        tile_t::get_pos(dir::left).y>
                 *)(hw_mem::west_or_self_tile_addr + hw_mem::tile_mem_beg_off);
  }

  /// Get the memory module on the right if it does exist
  auto &mem_east() {
    static_assert(is_memory_module_east(),
                  "There is no memory module"
                  " on the right of this tile in the right column and"
                  " on an odd row");
    return *(
        tile_mem_t<tile_t::get_pos(dir::right).x, tile_t::get_pos(dir::right).y>
            *)(hw_mem::east_or_self_tile_addr + hw_mem::tile_mem_beg_off);
  }

  /// Get the memory module below if it does exist
  auto &mem_south() {
    static_assert(is_memory_module_south(), "There is no memory module"
                                           " below the lower tile row");
    return *(
        tile_mem_t<tile_t::get_pos(dir::down).x, tile_t::get_pos(dir::down).y>
            *)(hw_mem::south_tile_addr + hw_mem::tile_mem_beg_off);
  }

  /// Get the memory module above if it does exist
  auto &mem_north() {
    static_assert(is_memory_module_north(), "There is no memory module"
                                         " above the upper tile row");
    return *(tile_mem_t<tile_t::get_pos(dir::up).x, tile_t::get_pos(dir::up).y>
                 *)(hw_mem::north_tile_addr + hw_mem::tile_mem_beg_off);
  }

  /// The memory module native to the tile
  auto &mem() {
    return *(
        tile_mem_t<tile_t::get_pos(dir::self).x, tile_t::get_pos(dir::self).y>
            *)(hw_mem::self_tile_addr(X, Y) + hw_mem::tile_mem_beg_off);
  }

  auto &mem_side() {
    if constexpr (Y & 1)
      return mem_west();
    else
      return mem_east();
  }

  device_lock get_lock(int8_t id, dir d = dir::self) { return {id + d * 16}; }

#endif
// TODO: Perhaps worth pushing all Lib X AI Engine functionallity we use down
// into a C++ API so it can all be excluded with one #IFDEF and kept nice and
// cleanly
// Part of the real current host -> device communication API using Lib X AI
// Engine
#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
  // for host side on device execution
  /// Store a way to access to hw tile instance
  void set_dev_handle(xaie::handle h) {
    TRISYCL_DUMP2(std::dec << "Mapping: (X = " << X << ", Y = " << Y
                            << ") to  (Row = " << (int)h.tile.Row
                            << ", Col = " << (int)h.tile.Col << ")", "exec");
    tb::set_dev_handle(h);
  }
  xaie::handle get_dev_handle() {
    return tb::get_dev_handle();
  }
#endif

  /// The type of the memory module native to the tile
  using mem_t = typename AIE_Program::template tileable_memory<x, y>;


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
  template <typename T>
  auto get_cascade_stream_in() {
    static_assert(!is_cascade_start(), "You cannot access to the cascade stream"
                  " input on the tile that starts the stream");
    return tb::cascade().template get_cascade_stream_in<T>(x, y);
  }


  /** Get a write accessor to the cascade stream output

      \param T is the data type used to write to the cascade
      stream pipe

      \param Target is the access mode to the pipe. It is blocking
      by default
  */
  template <typename T>
  auto get_cascade_stream_out() {
    static_assert(!is_cascade_end(), "You cannot access to the cascade stream"
                  " output on the tile that starts the stream");
    return tb::cascade().template get_cascade_stream_out<T>(x, y);
  }


  /** An horizontal barrier using a lock

      Implement a barrier across the tiles a line.

      \param[in] lock is the ME lock to use. The lock 14 is used by
      default
  */
  void horizontal_barrier(int lock = 14) {
    if constexpr (y & 1) {
      // Propagate a token from West to East and back
      if constexpr (!is_west_column()) {
        // Wait for the Western neighbour to be ready
        mem().lock(lock).acquire_with_value(true);
      }
      if constexpr (is_memory_module_east()) {
        mem_east().lock(lock).acquire_with_value(false);
        // Unleash the Eastern neighbour
        mem_east().lock(lock).release_with_value(true);
        // Wait for the Eastern neighbour to acknowledge
        mem_east().lock(lock).acquire_with_value(false);
       }
      if constexpr (!is_west_column()) {
        // Acknowledge to the Western neighbour
        mem().lock(lock).release_with_value(false);
      }
    } else {
      // Propagate a token from East to West and back
      if constexpr (!is_east_column()) {
        // Wait for the Eastern neighbour to be ready
        mem().lock(lock).acquire_with_value(true);
      }
      if constexpr (is_memory_module_west()) {
        mem_west().lock(lock).acquire_with_value(false);
        // Unleash the Western neighbour
        mem_west().lock(lock).release_with_value(true);
        // Wait for the Western neighbour to acknowledge
        mem_west().lock(lock).acquire_with_value(false);
       }
      if constexpr (!is_east_column()) {
        // Acknowledge to the Eastern neighbour
        mem().lock(lock).release_with_value(false);
      }
    }
  }


  /** A vertical barrier using a lock

      Implement a barrier across the tiles a line.

      \param[in] lock is the ME lock to use. The lock 15 is used by
      default
  */
  void vertical_barrier(int lock = 15) {
    // Propagate a token from South to North and back
    if constexpr (!is_south_row()) {
      // Wait for the Southern neighbour to be ready
      mem().lock(lock).acquire_with_value(true);
    }
    if constexpr (is_memory_module_north()) {
      mem_north().lock(lock).acquire_with_value(false);
      // Unleash the Northern neighbour
      mem_north().lock(lock).release_with_value(true);
      // Wait for the Northern neighbour to acknowledge
      mem_north().lock(lock).acquire_with_value(false);
    }
    if constexpr (!is_south_row()) {
      // Acknowledge to the Southern neighbour
      mem().lock(lock).release_with_value(false);
    }
  }


  /** Full barrier using the 2 locks by default

      Implement a barrier across the full program by using \c
      horizontal_barrier() and \c vertical_barrier().
  */
  void barrier() {
    horizontal_barrier();
    vertical_barrier();
  }


  /** Get access on a receiver DMA

      \param[in] port specifies which DMA to access, starting at 0 */
  auto rx_dma(int port) {
    return dma_dsl { *this, this->ti.rx_dma(port) };
  }


  /** Get access on a transmit DMA

      \param[in] port specifies which DMA to access, starting at 0 */
  auto tx_dma(int port) {
    return dma_dsl { *this, this->ti.tx_dma(port) };
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
