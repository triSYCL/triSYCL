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
#include "xaie_wrapper.hpp"
#include "hardware.hpp"
#include "program.hpp"
#include "lock.hpp"
#include "log.hpp"

#ifdef __SYCL_XILINX_AIE__
/// When executing on real hardware on the host or device side.

#include "exec_kernel.hpp"
#include "hardware.hpp"
#include "lock.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

template<typename Geo, typename CRTP>
struct tile_hw_impl {
  CRTP& get() { return *(CRTP*)this; }
  const CRTP& get() const { return *(const CRTP*)this; }

  template<typename D, typename P>
  void initialize(D& device, P) {
#if !defined(__SYCL_DEVICE_ONLY__)
    /// We do not need the program so we just ignore it.
    dev_handle = xaie::handle{xaie::acap_pos_to_xaie_pos(CRTP::self_position),
                              device.get_dev_inst()};
#else
    assert(false && "should never be executed on device");
#endif
  }

#if !defined(__SYCL_DEVICE_ONLY__)
  // For host side when executing on acap hardware
  xaie::handle dev_handle;

  xaie::handle get_dev_handle() {
    return dev_handle;
  }
#else
  xaie::handle get_dev_handle() {
    assert(false && "should never be executed on device");
    return {};
  }
#endif

  /// Submit a callable on this tile
  template <typename Work> void single_task(Work &&f) {
    // detail::tile_infrastructure<typename device::geo>{}.single_task(std::forward<Work>(f));
    detail::exec_kernel<CRTP>{}.exec(get_dev_handle(), std::forward<Work>(f));
  }

  template <hw::dir d> auto &dir_mem() {
#if !defined(__SYCL_DEVICE_ONLY__)
    assert(false && "This should never be executed on the host");
#endif
    constexpr uint32_t tile_addr = get_base_addr(d);
    return *(typename CRTP::template tile_mem_t<CRTP::self_position.moved(d)>
                 *)(tile_addr + hw::tile_mem_begin_offset);
  }

#if !defined(__SYCL_DEVICE_ONLY__)
  /// Get the horizontal coordinate
  int x_coord() { return CRTP::x; }

  /// Get the vertical coordinate
  int y_coord() { return CRTP::y; }
#else
  /// Get the horizontal coordinate
  int x_coord() { return hw::get_tile_x_coordinate(); }

  /// Get the vertical coordinate
  int y_coord() { return hw::get_tile_y_coordinate(); }
#endif

  hw_lock get_lock(hw::dir d, int i) {
#if !defined(__SYCL_DEVICE_ONLY__)
    return {i, dev_handle.moved(CRTP::self_position.moved(d))};
#else
    return {d, i};
#endif
  }

  /// write 48 byte starting from ptr to the cascade stream
  void cascade_stream_write48(const char *ptr) {
    acap_intr::cstream_write48(ptr);
  }

  /// read 48 byte into ptr to the cascade stream
  void cascade_stream_read48(char *ptr) {
    acap_intr::cstream_read48(ptr);
  }

  void stream_write16(const char *ptr, int stream_dix) {
    acap_intr::stream_write16(ptr, stream_dix);
  }

  void stream_read16(char *ptr, int stream_dix) {
    acap_intr::stream_read16(ptr, stream_dix);
  }

  /// When waiting on the host we should go through this function but throught
  /// the wait_all. and we should not each this function either on the device.
  void wait() { assert(false && "unreachable"); }


  /** Get the user input connection from the AXI stream switch

      \param[in] port is the port to use
  */
  auto &in_connection(int port) {
    assert(false && "not yet implemented");
  }

  /** Get the user output connection to the AXI stream switch

      \param[in] port is port to use
  */
  auto &out_connection(int port) {
    assert(false && "not yet implemented");
  }

  /// Get the user input port from the AXI stream switch
  auto &in(int port) {
    assert(false && "not yet implemented");
  }

  /// Get the user output port to the AXI stream switch
  auto &out(int port) {
    assert(false && "not yet implemented");
  }
};

template<typename T1, typename T2, typename T3>
using tile_backend = tile_hw_impl<T1, T3>;
}
#else
/// When executing in the emulator.

#include "tile_infrastructure.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

template <typename Geo, typename AIE_Program, typename CRTP>
struct tile_emu_impl : tile_infrastructure<Geo> {
  using base = tile_infrastructure<Geo>;
  CRTP &get() { return *(CRTP *)this; }
  const CRTP &get() const { return *(const CRTP *)this; }

  template<typename D, typename P>
  void initialize(D& device, P& prog) {
    program = &prog;
    base::set_tile_infrastructure(device.tile(CRTP::self_position.x , CRTP::self_position.y));
  }

  /// Keep a reference to the AIE_Program with the full tile and memory view
  AIE_Program *program;

  /// Get the horizontal coordinate
  int x_coord() { return CRTP::x; }

  /// Get the vertical coordinate
  int y_coord() { return CRTP::y; }

  template <hw::dir d> auto &dir_mem() {
    return program->template memory_module<CRTP::memory_module_linear_id(
        hw::get_simple_offset(d).x, hw::get_simple_offset(d).y)>();
  }

  /// read 48 byte into ptr to the cascade stream
  void cascade_stream_read48(char *ptr) { return base::cascade().read48(ptr); }

  /// write 48 byte starting from ptr to the cascade stream
  void cascade_stream_write48(const char *ptr) {
    hw::position next_in_cascade = {
        Geo::cascade_linear_x(Geo::cascade_linear_id(CRTP::x, CRTP::y) + 1),
        Geo::cascade_linear_y(Geo::cascade_linear_id(CRTP::x, CRTP::y) + 1)};
    program->tile_infra(next_in_cascade.x, next_in_cascade.y)
        .cascade()
        .write48(ptr);
  }

  void stream_write16(const char *ptr, int stream_dix) {
    acap_intr::stream_write16(ptr, stream_dix);
  }

  void stream_read16(char *ptr, int stream_dix) {
    acap_intr::stream_read16(ptr, stream_dix);
  }

  lock_unit &get_lock(hw::dir d, int i) {
    program->tile_infra(CRTP::self_position.moved(d)).get_lock(i);
  }
};

template<typename ... Ts>
using tile_backend = tile_emu_impl<Ts...>;
}
#endif

/// TODO: Perhaps worth pushing all LibXAiengine functionallity we use down
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
struct tile : public tile_backend<typename AIE_Program::geo, AIE_Program, tile<AIE_Program, X, Y>> {
  using base = tile_backend<typename AIE_Program::geo, AIE_Program, tile<AIE_Program, X, Y>>;

  /** The horizontal tile coordinates in the CGRA grid (starting at 0
      and increasing towards the East) */
  static auto constexpr x = X;
  /** The vertical tile coordinates in the CGRA grid (starting at
      increasing towards the North) */
  static auto constexpr y = Y;

  /// The geography of the CGRA
  using geo = typename AIE_Program::geo;
  using impl = base;
  using prog = AIE_Program;

  /// The type of the memory module of a tile at offset (dx, dy) from the
  /// current tile.
  template <hw::position p>
  using tile_mem_t = typename AIE_Program::template tileable_memory<p.x, p.y>;

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

  static void log(auto i) {
    acap::log(i);
  }

  static constexpr acap::hw::position self_position{X, Y};

  /// Get the memory module on the left if it does exist
  auto &mem_west() {
    static_assert(is_memory_module_west(),
                  "There is no memory module"
                  " on the left of this tile in the left column and"
                  " on an even row");
    return base::template dir_mem<hw::dir::west>();
  }

  /// Get the memory module on the right if it does exist
  auto &mem_east() {
    static_assert(is_memory_module_east(),
                  "There is no memory module"
                  " on the right of this tile in the right column and"
                  " on an odd row");
    return base::template dir_mem<hw::dir::east>();
  }

  /// Get the memory module below if it does exist
  auto &mem_south() {
    static_assert(is_memory_module_south(), "There is no memory module"
                                            " below the lower tile row");
    return base::template dir_mem<hw::dir::south>();
  }

  /// Get the memory module above if it does exist
  auto &mem_north() {
    static_assert(is_memory_module_north(), "There is no memory module"
                                            " above the upper tile row");
    return base::template dir_mem<hw::dir::north>();
  }

  /// The memory module native to the tile
  auto &mem() {
    if constexpr (self_position.get_parity() == hw::parity::west)
      return mem_west();
    else
      return mem_east();
  }

  auto &mem_side() {
    if constexpr (self_position.get_parity() == hw::parity::east)
      return mem_west();
    else
      return mem_east();
  }

  /** Get the memory module relative to the tile

      \param[in] Dx is the horizontal offset relative to the current core tile

      \param[in] Dy is the vertical offset relative to the current core tile

      \return the memory tile object

      Note that since a core tile has a 4-neighbor connectivity, at
      least one of the offset needs to be 0 and the other to be either
      1 or -1.
  */
  template <int Dx, int Dy>
  auto &mem() {
    static_assert(geo::is_valid_memory_module_offset(Dx, Dy),
                  "Note that since a core tile has a 4-neighbor connectivity,"
                  " one of the offset needs to be 0 and the other to be either"
                  " 1 or -1");
    if constexpr (Dx == -1)
      return mem_west();
    else if constexpr (Dx == 1)
      return mem_east();
    else if constexpr (Dy == -1)
      return mem_south();
    else
      return mem_north();
  }


  using dir = hw::dir;
  using base::get_lock;

  auto get_lock(int i) {
    if constexpr (self_position.get_parity() == hw::parity::west)
      return get_lock(hw::dir::west, i);
    else
      return get_lock(hw::dir::east, i);
  }

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

private:
  /// Generic function to deal with fixed size stream operations.
  template <unsigned OpSize, bool is_read, typename OpFunc>
  static void stream_operation(typename std::conditional<is_read, char, const char>::type *ptr,
                        unsigned size, OpFunc func) {
    std::array<char, OpSize> buffer;
    buffer.fill(0);
    unsigned idx = 0;
    for (; idx + OpSize <= size; idx += OpSize)
      func(ptr + idx);
    if (idx < size) {
      if constexpr (!is_read)
        std::memcpy(buffer.data(), ptr + idx, size - idx);
      func(buffer.data());
      if constexpr (is_read)
        std::memcpy(ptr + idx, buffer.data(), size - idx);
    }
  }

public:
  /** Write an object of type SpecifiedTy to a cascade stream

      \param SpecifiedTy is the data type used to read from the cascade
      stream pipe
  */
  template <typename SpecifiedTy> void cascade_write(const SpecifiedTy &value) {
    static_assert(!is_cascade_end(), "There is nothing to write for.");
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    stream_operation<48, /*is_read*/ false>(
        reinterpret_cast<const char *>(std::addressof(value)),
        sizeof(SpecifiedTy),
        [this](const char *ptr) { base::cascade_stream_write48(ptr); });
  }

  /** Read an object of type SpecifiedTy from the cascade stream

      \param SpecifiedTy is the data type used to write to the cascade
      stream pipe
  */
  template <typename SpecifiedTy> SpecifiedTy cascade_read() {
    static_assert(!is_cascade_start(), "There is nothing to read from");
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    SpecifiedTy value;
    stream_operation<48, /*is_read*/ true>(
        reinterpret_cast<char *>(std::addressof(value)), sizeof(SpecifiedTy),
        [this](char *ptr) { base::cascade_stream_read48(ptr); });
    return value;
  }

  template <typename SpecifiedTy>
  void stream_write(const SpecifiedTy &value, int stream_id) {
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    stream_operation<16, /*is_read*/ false>(
        reinterpret_cast<const char *>(std::addressof(value)),
        sizeof(SpecifiedTy), [this, stream_id](const char *ptr) {
          base::stream_write16(ptr, stream_id);
        });
  }

  /** Read an object of type SpecifiedTy from the stream

      \param SpecifiedTy is the data type used to write to the cascade
      stream pipe
  */
  template <typename SpecifiedTy> SpecifiedTy stream_read(int stream_id) {
    static_assert(std::is_trivially_copyable<SpecifiedTy>::value,
                  "SpecifiedTy cannot passed by streams");
    SpecifiedTy value;
    stream_operation<16, /*is_read*/ true>(
        reinterpret_cast<char *>(std::addressof(value)), sizeof(SpecifiedTy),
        [this, stream_id](char *ptr) { base::stream_read16(ptr, stream_id); });
    return value;
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
        get_lock(lock).acquire_with_value(true);
      }
      if constexpr (is_memory_module_east()) {
        get_lock(hw::dir::east, lock).acquire_with_value(false);
        // Unleash the Eastern neighbour
        get_lock(hw::dir::east, lock).release_with_value(true);
        // Wait for the Eastern neighbour to acknowledge
        get_lock(hw::dir::east, lock).acquire_with_value(false);
       }
      if constexpr (!is_west_column()) {
        // Acknowledge to the Western neighbour
        get_lock(lock).release_with_value(false);
      }
    } else {
      // Propagate a token from East to West and back
      if constexpr (!is_east_column()) {
        // Wait for the Eastern neighbour to be ready
        get_lock(lock).acquire_with_value(true);
      }
      if constexpr (is_memory_module_west()) {
        get_lock(hw::dir::west, lock).acquire_with_value(false);
        // Unleash the Western neighbour
        get_lock(hw::dir::west, lock).release_with_value(true);
        // Wait for the Western neighbour to acknowledge
        get_lock(hw::dir::west, lock).acquire_with_value(false);
       }
      if constexpr (!is_east_column()) {
        // Acknowledge to the Eastern neighbour
        get_lock(lock).release_with_value(false);
      }
    }
    /// Reset the lock for the next barrier.
    get_lock(lock).release_with_value(false);
  }


  /** A vertical barrier using a lock

      Implement a barrier across the tiles a line.

      \param[in] lock is the ME lock to use. The lock 15 is used by
      default
  */
  void vertical_barrier(int lock = 15) {
    // Propagate a token from South to North and back
    // All tile except the bottom one wait.
    if constexpr (!is_south_row()) {
      // Wait for the Southern neighbour to be ready
      get_lock(lock).acquire_with_value(true);
    }
    // All tile except the top one wait.
    if constexpr (is_memory_module_north()) {
      get_lock(hw::dir::north, lock).acquire_with_value(false);
      // Unleash the Northern neighbour
      get_lock(hw::dir::north, lock).release_with_value(true);
      // Wait for the Northern neighbour to acknowledge
      get_lock(hw::dir::north, lock).acquire_with_value(false);
    } 
    // All tile except the bottom one wait.
    if constexpr (!is_south_row()) {
      // Acknowledge to the Southern neighbour
      get_lock(lock).release_with_value(false);
    }
    /// Reset the lock for the next barrier.
    get_lock(lock).release_with_value(false);
  }

  /** Full barrier using the 2 locks by default

      Implement a barrier across the full program by using \c
      horizontal_barrier() and \c vertical_barrier().
  */
  void barrier(int h_id = 14, int v_id = 15) {
    horizontal_barrier(h_id);
    vertical_barrier(v_id);
  }

/// not yet implemented
#if 0
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
#endif

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
