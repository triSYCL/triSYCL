#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_BASE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_BASE_HPP

/** \file

    The basic AI Engine homogeneous tile, with common content to all
    the tiles (i.e. independent of x & y coordinates) but depending on
    a collective program.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "tile_infrastructure.hpp"
#include "xaie_wrapper.hpp"

/// TODO: Perhaps worth pushing all Lib X AI Engine functionallity we use down
/// into a C++ API so it can all be excluded with one #ifdef and kept nice and
/// cleanly

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

#ifdef __SYCL_DEVICE_ONLY__

/** The AI Engine device tile infrastructure common to all the tiles when
    compilied inside of a kernel for device.

    Similar to the tile device class this is an incremental WIP approach to
    include the desired Tile functionallity on device but essentially this will
    end up as some kind of tile_base Lite as we work out what works on device
    and what doesn't. For example std::thread should not be part of this base
    class and things like functor inheritance seem a little unstable at the
    moment. Also components of the STL.h can leak in which are not compatible
    with the device compilation as SYCL has some rules that prevent things like
    on device memory intialization, this is a problem when accessors leak in as
    they make use of the STL and memory.h which will kill compilation.

    In the long term it's possible this will end up very similar to tile_base in
    which case they can be merged with optionally modifications for removing
    std::thread etc on device

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents
*/
template <typename AIE_Program>
class tile_base {

  using device = typename AIE_Program::device;

public:

  /// Noop on device, it has to exist for compilation purposes as Host side code
  /// is still compiled for the device unfortunately.
  void set_program(AIE_Program &p) {}

  /// Routines to run before core starts running.
  int prerun() { return 1; }

  /** Provide a run member function that does nothing so it is
      possible to write a minimum AI Engine program that does nothing.

      Note that even if this function is not virtual, in the common
      case a programmer implements it to specify the program executed
      by a tile
  */
  void run() {}

  /// Routines to run after core completes running.
  void postrun() {}

  template <typename Work> void single_task(Work &&f) {
    detail::tile_infrastructure<typename device::geo>{}.single_task(std::forward<Work>(f));
  }

  void set_tile_infrastructure(const tile_infrastructure<device> &t) {}
};

#endif // ifdef SYCL_DEVICE_ONLY

#ifndef __SYCL_DEVICE_ONLY__

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param AIE_Program is the type representing the full CGRA program
    with the tile programs and memory contents
*/
template <typename AIE_Program>
class tile_base {

  using geo = typename AIE_Program::device::geo;

protected:

  /// Keep a reference to the AIE_Program with the full tile and memory view
  /// \todo can probably be removed
  AIE_Program *program;

  /// Keep a reference to the tile_infrastructure hardware features
  tile_infrastructure<geo> ti;

public:

#ifdef __SYCL_XILINX_AIE__
  /// Store a way to access to hw tile instance
  void set_dev_handle(xaie::handle h) {
    ti.set_dev_handle(h);
  }
  xaie::handle get_dev_handle() const {
    return ti.get_dev_handle();
  }
#endif

  /// Routines to run before core starts running.
  int prerun() {
    return 1;
  }

  /** Provide a run member function that does nothing so it is
      possible to write a minimum AI Engine program that does nothing.

      Note that even if this function is not virtual, in the common
      case a programmer implements it to specify the program executed
      by a tile
  */
  void run() {
  }

  /// Routines to run after core completes running.
  void postrun() {
  }

  /// Submit a callable on this tile
  template <typename Work>
  void single_task(Work &&f) {
    ti.single_task(std::forward<Work>(f));
  }


  /// Wait for the execution of the callable on this tile
  void wait() {
    ti.wait();
  }


  /// Access the cascade connections
  auto &cascade() {
    // \todo the cascade should be part of the tile infrastructure instead
    return program->cascade();
  }


  /** Get the user input connection from the AXI stream switch

      \param[in] port is the port to use
  */
  auto &in_connection(int port) {
    return ti.in_connection(port);
  }


  /** Get the user output connection to the AXI stream switch

      \param[in] port is port to use
  */
  auto &out_connection(int port) {
    return ti.out_connection(port);
  }


  /// Get the user input port from the AXI stream switch
  auto &in(int port) {
    return ti.in(port);
  }


  /// Get the user output port to the AXI stream switch
  auto &out(int port) {
    return ti.out(port);
  }


  /// Store a way to access to the program
  void set_program(AIE_Program &p) {
    program = &p;
  }

  /// Store a way to access to hardware infrastructure of the tile
  void set_tile_infrastructure(const tile_infrastructure<geo> t) {
    ti = t;
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_BASE_HPP