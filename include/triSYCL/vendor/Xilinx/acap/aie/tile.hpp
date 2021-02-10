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

#include "triSYCL/access.hpp"
#include "triSYCL/detail/program_manager.hpp"
#include "tile_base.hpp"
#include "aie_utils.hpp"


/// TODO: Perhaps worth pushing all Lib X AI Engine functionallity we use down
/// into a C++ API so it can all be excluded with one #ifdef and kept nice and
/// cleanly
#ifdef __SYCL_XILINX_AIE__
extern "C" {
  #include <xaiengine.h>
}
#endif

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

#ifdef __SYCL_DEVICE_ONLY__

/** An empty device tile for now, what is enabled for a tile inside of a Kernel
    the aim is to incrementally add on top of this without breaking the kernel
    compilation for now.

    It's an incremental WIP approach but essentially this will end up as some
    kind of Tile Lite as we work out what works on device and what doesn't.

    For example does having the X AI Engine components on the device make sense?
    I imagine not as they seem to be host only...

    The Intel SYCL runtime has a similar concept with some of it's APIs like
    accessors, so perhaps it makes sense for Tiles.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents

    \param X is the horizontal coordinate of the memory module

    \param Y is the vertical coordinate of the memory module

*/
template <typename AIE_Program, int X, int Y>
struct tile : tile_base<AIE_Program> {
  /** The horizontal tile coordinates in the CGRA grid (starting at 0
      and increasing to the right) */
  static auto constexpr x = X;
  /** The vertical tile coordinates in the CGRA grid (starting at
      increasing to the top) */
  static auto constexpr y = Y;

  /// Noop on device, it has to exist for compilation purposes as Host side code
  /// is still compiled for the device unfortunately.
  std::uint32_t mem_read(std::uint32_t offset) { return 0; }

  /// Noop on device, it has to exist for compilation purposes as Host side code
  /// is still compiled for the device unfortunately.
  void mem_write(std::uint32_t offset, std::uint32_t data) {}

};

#endif // ifdef SYCL_DEVICE_ONLY

#ifndef __SYCL_DEVICE_ONLY__

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
      and increasing to the right) */
  static auto constexpr x = X;
  /** The vertical tile coordinates in the CGRA grid (starting at
      increasing to the top) */
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
     return tb::program->template
       memory_module<memory_module_linear_id(-1, 0)>();
  }


  /// Get the memory module on the right if it does exist
  auto &mem_right() {
    static_assert(is_memory_module_right(), "There is no memory module"
                  " on the right of this tile in the right column and"
                   " on an odd row");
    return tb::program->template
      memory_module<memory_module_linear_id(1, 0)>();
  }


  /// Get the memory module below if it does exist
  auto &mem_down() {
    static_assert(is_memory_module_down(), "There is no memory module"
                  " below the lower tile row");
    return tb::program->template
      memory_module<memory_module_linear_id(0, -1)>();
  }


  /// Get the memory module above if it does exist
  auto &mem_up() {
    static_assert(is_memory_module_up(), "There is no memory module"
                  " above the upper tile row");
    return tb::program->template
      memory_module<memory_module_linear_id(0, 1)>();
  }


  /// The memory module native to the tile
  auto &mem() {
    if constexpr (y & 1)
      return mem_left();
    else
      return mem_right();
  }

// TODO: Perhaps worth pushing all Lib X AI Engine functionallity we use down
// into a C++ API so it can all be excluded with one #IFDEF and kept nice and
// cleanly
// Part of the real current host -> device communication API using Lib X AI
// Engine
#ifdef __SYCL_XILINX_AIE__
  /// The memory read accessors
  std::uint32_t mem_read(std::uint32_t offset) {
    std::uint32_t Data;
    TRISYCL_XAIE(
        XAie_DataMemRdWord(tb::aie_inst, tb::aie_hw_tile, offset, &Data));
    TRISYCL_DUMP_T("Reading: (" << X << ", " << Y << ") + 0x" << std::hex
                              << offset << " = " << std::dec << Data);
    return Data;
  }

  /// The memory write accessors
  void mem_write(std::uint32_t offset, std::uint32_t data) {
    TRISYCL_XAIE(
        XAie_DataMemWrWord(tb::aie_inst, tb::aie_hw_tile, offset, data));
    TRISYCL_DUMP_T("Writing: (" << X << ", " << Y << ") + 0x" << std::hex
                               << offset << " = " << std::dec << data);
  }

  /// Configure device for dma.
  void mem_dma(uint32_t offset, uint32_t size) {
    XAie_DmaDesc DmaDesc;
    TRISYCL_XAIE(XAie_DmaDescInit(tb::aie_inst, &DmaDesc, tb::aie_hw_tile));
    TRISYCL_XAIE(XAie_DmaSetAddrLen(&DmaDesc, offset, size));
    TRISYCL_DUMP_T("Setup Dma: (" << X << ", " << Y << ") 0x" << std::hex
                                << offset << "-0x" << (offset + size));
  }

  /// FIXME: is this right location for these functions?
  /// Load the elf via path to a elf binary file, handy for debugging if you
  /// wish to dump a stored binary or load something AoT compiled seperately
  void load_elf_file(const char *path) {
    // global load of elf
    TRISYCL_XAIE(XAie_LoadElf(tb::aie_inst, tb::aie_hw_tile, path, 0));
  }

  /// Load the elf via path to a block of memory which contains an elf image
  void load_elf_image(std::string image) {
    assert(detail::program_manager::isELFMagic(image.data()) &&
           "invalid elf magic");
    TRISYCL_XAIE(XAie_LoadElfMem(tb::aie_inst, tb::aie_hw_tile,
                                 reinterpret_cast<u8 *>(image.data())));
  }

  /// Reset the core
  void core_reset() {
    TRISYCL_XAIE(XAie_CoreUnreset(tb::aie_inst, tb::aie_hw_tile));
  }

  /// Start the core
  void core_run() {
    TRISYCL_XAIE(XAie_CoreEnable(tb::aie_inst, tb::aie_hw_tile));
  }

  /// Stop the core
  void core_stop() {
    TRISYCL_XAIE(XAie_CoreDisable(tb::aie_inst, tb::aie_hw_tile));
  }

  /// Wait for the core to complete
  void core_wait() {
    TRISYCL_DUMP_T("(" << X << ", " << Y << ") Waiting for kernel...");
    AieRC RC = XAIE_OK;
    do {
      RC = XAie_CoreWaitForDone(tb::aie_inst, tb::aie_hw_tile, 0);
    } while (RC != XAIE_OK);
    TRISYCL_DUMP_T("(" << X << ", " << Y << ") done");
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
      // Propagate a token from left to right and back
      if constexpr (!is_left_column()) {
        // Wait for the left neighbour to be ready
        mem().lock(lock).acquire_with_value(true);
      }
      if constexpr (is_memory_module_right()) {
        mem_right().lock(lock).acquire_with_value(false);
        // Unleash the right neighbour
        mem_right().lock(lock).release_with_value(true);
        // Wait for the right neighbour to acknowledge
        mem_right().lock(lock).acquire_with_value(false);
       }
      if constexpr (!is_left_column()) {
        // Acknowledge to the left neighbour
        mem().lock(lock).release_with_value(false);
      }
    } else {
      // Propagate a token from right to left and back
      if constexpr (!is_right_column()) {
        // Wait for the right neighbour to be ready
        mem().lock(lock).acquire_with_value(true);
      }
      if constexpr (is_memory_module_left()) {
        mem_left().lock(lock).acquire_with_value(false);
        // Unleash the left neighbour
        mem_left().lock(lock).release_with_value(true);
        // Wait for the left neighbour to acknowledge
        mem_left().lock(lock).acquire_with_value(false);
       }
      if constexpr (!is_right_column()) {
        // Acknowledge to the right neighbour
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
    // Propagate a token from bottom to top and back
    if constexpr (!is_bottom_row()) {
      // Wait for the neighbour below to be ready
      mem().lock(lock).acquire_with_value(true);
    }
    if constexpr (is_memory_module_up()) {
      mem_up().lock(lock).acquire_with_value(false);
      // Unleash the neighbour above
      mem_up().lock(lock).release_with_value(true);
      // Wait for the neighbour above to acknowledge
      mem_up().lock(lock).acquire_with_value(false);
    }
    if constexpr (!is_bottom_row()) {
      // Acknowledge to the neighbour below
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

};

#endif // ifndef SYCL_DEVICE_ONLY

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
