#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_PROGRAM_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_PROGRAM_HPP

/** \file

    Model of an AI Engine program, that weaves the program of each tile
    with the memory of each tile for a given device

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>
#include <type_traits>

#include "connection.hpp"
#include "geography.hpp"
#include "memory.hpp"
#include "rpc.hpp"
#include "tile.hpp"
#include "rpc.hpp"
#include "xaie_wrapper.hpp"
#include "triSYCL/detail/program_manager.hpp"
#include "triSYCL/detail/kernel_desc.hpp"

/// TODO: Perhaps worth pushing all LibXAiengine functionallity we use down
/// into a C++ API so it can all be excluded with one #ifdef and kept nice and
/// cleanly

/// \ingroup acap
///  @{

/** \defgroup aie AI Engine CGRA

    Extensions to support explicit AI Engine system-wide programming in C++
    @{
*/

namespace trisycl::vendor::xilinx::acap::aie {

/** Define an AI Engine CGRA program with its code and memory per core

    \param AIEDevice is the device description of the machine to
    instantiate with the physical size

    \param Tile is the description of the program tiles to
    instantiate. By default each tile will run an empty program.

    \param Memory is the description of the machine memory modules. By
    default the machine has empty memory modules.
*/
template <typename AIEDevice,
          template <typename AIE,
                    int X,
                    int Y> typename Tile = acap::aie::tile,
          template <typename AIE,
                    int X,
                    int Y> typename Memory = acap::aie::memory>
struct program {

  /// The geography of the CGRA
  using geo = typename AIEDevice::geo;
  using device = AIEDevice;

  /// The device running this program
  AIEDevice aie_d;


#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  rpc::host_side rpc_system;
#endif

  /// Type describing all the memory modules of the CGRA
  template <int X, int Y>
  using tileable_memory = Memory<program, X, Y>;

  /** The tiled memory modules of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_memory>())
  memory_modules = geo::template generate_tiles<tileable_memory>();

  /// Type describing the programs of all the cores in the CGRA
  template <int X, int Y>
  using tileable_tile = Tile<program, X, Y>;

  /** The tiled programs of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_tile>()) tiles =
    geo::template generate_tiles<tileable_tile>();

  /** Access to a heterogeneous memory module by its linear id

      \param[in] LinearId is the linear id
  */
  template <int LinearId>
  auto &memory_module() {
    return boost::hana::at_c<LinearId>(memory_modules);
  }


  /** Access to a heterogeneous memory module by its coordinates

      \param[in] X is the horizontal memory module coordinate

      \param[in] Y is the vertical memory module coordinate
  */
  template <int X, int Y>
  auto &memory_module() {
    return memory_module<geo::linear_id(X, Y)>();
  }

  /** Access to a heterogeneous tile by linear id

      \param[in] LinearId is the linear id
  */
  template <int LinearId>
  auto &tile() {
    return boost::hana::at_c<LinearId>(tiles);
  }


  /** Access to a heterogeneous tile by its coordinates

      \param[in] X is the horizontal tile coordinate

      \param[in] Y is the vertical tile coordinate
  */
  template <int X, int Y>
  auto &tile() {
    return tile<geo::linear_id(X, Y)>();
  }

  /** Access to a tile_infrastructure by its coordinates

      \param[in] X is the horizontal tile coordinate

      \param[in] Y is the vertical tile coordinate
  */
  auto& tile_infra(int x, int y) {
    return aie_d.tile(x, y);
  }

  /** Iterate on all the tile bases of the AIE in an homogeneous way

      \param[in] F is the function to apply on each tile base
  */
  template <typename F> void for_each_tile_infra(F &&f) {
    for (auto y = 0; y != geo::y_size; ++y)
      for (auto x = 0; x != geo::x_size; ++x)
        f(aie_d.tile(x, y));
  }

  /// Create the AIE program with the tiles and memory modules
  program(AIEDevice &aie_d) : aie_d { aie_d }
#if !defined(__SYCL_DEVICE_ONLY__) && defined(__SYCL_XILINX_AIE__)
  , rpc_system {
    geo::x_size, geo::y_size, xaie::handle({0, 0}, aie_d.get_dev_inst()),
  }
#endif
  {
    // Initialization of the AI Engine tile constructs from LibXAiengine

    boost::hana::for_each(tiles, [&](auto &t) {
      t.initialize(aie_d, *this);
    });
  }

  /// Wait for the end of the execution of each tile
  void wait() {
    /// There the device doesn't need to wait for itself to execute.
#if !defined(__SYCL_DEVICE_ONLY__)
#if defined(__SYCL_XILINX_AIE__)
    TRISYCL_DUMP2("Joining AIE tiles...", "exec");
    rpc_system.wait_all();
    TRISYCL_DUMP2("Joined AIE tiles", "exec");
    boost::hana::for_each(tiles, [&](auto &t) {     
      if constexpr (requires { t.postrun(); })
        t.postrun();
    });
#else
    boost::hana::for_each(tiles, [&](auto &t) {
      TRISYCL_DUMP2("Joining AIE tile (" << t.x << ',' << t.y << ')', "exec");
      t.wait();
      TRISYCL_DUMP2("Joined AIE tile (" << t.x << ',' << t.y << ')', "exec");
    });
#endif
#endif
  }

  void lock() {
    boost::hana::for_each(tiles, [&](auto &t) {
      if constexpr (requires { t.lock(); })
        t.lock();
    });
  }

  /** Launch the programs of all the tiles of the CGRA in their own
      executor (CPU thread, fiber...) and wait for their completion.

      This is the main member function to use to launch the execution.
  */
  void run() {
    lock();
    // Start each tile program
    boost::hana::for_each(tiles, [&](auto &t) {
      t.single_task(t);
    });
    wait();
  }
  /** Run synchronously an heterogeneous invocable collectively on the device

      \param f is an invocable taking an heterogeneous tile handler

      \todo Factorize out the 2 run functions
  */
  template <typename Invocable> void run(Invocable &&f) {
    lock();
    // Start each tile program in its own executor
    boost::hana::for_each(
        tiles, [&](auto &t) { t.single_task(std::forward<Invocable>(f)); });
    wait();
  }
};

/// @} End the aie Doxygen group
/// @} End the acap Doxygen group

}

/*
s    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_PROGRAM_HPP
