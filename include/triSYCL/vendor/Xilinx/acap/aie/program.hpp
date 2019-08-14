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
#include "memory_base.hpp"
#include "tile.hpp"
#include "tile_base.hpp"

extern "C" {
  #include <xaiengine.h>
}

/// \ingroup acap
///  @{

/** \defgroup aie AI Engine CGRA

    Extensions to support explicit AI Engine system-wide programming in C++
    @{
*/

class name_for_kernel;

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
  AIEDevice &aie_d;

  /// Type describing all the memory modules of the CGRA
  template <int X, int Y>
  using tileable_memory = Memory<program, X, Y>;

  /** The tiled memory modules of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_memory>())
  memory_modules = geo::template generate_tiles<tileable_memory>();

  /** Keep track of all the tiled memory modules as a type-erased
      memory_modules_base type to have a simpler access to the basic
      position-independent memory module features */
  memory_base *memory_modules_bases[geo::y_size][geo::x_size];

  /// Type describing the programs of all the cores in the CGRA
  template <int X, int Y>
  using tileable_tile = Tile<program, X, Y>;

  /** The tiled programs of the CGRA

      Unfortunately it is not possible to use \c auto here...
      Otherwise it could be just: \code static inline auto \endcode */
  decltype(geo::template generate_tiles<tileable_tile>()) tiles =
    geo::template generate_tiles<tileable_tile>();

  /** Keep track of all the tiles as a type-erased tile_base type to
      have a simpler access to the basic position-independent tile
      features */
  tile_base<program> *tile_bases[geo::y_size][geo::x_size];

  /** Access to the common infrastructure part of a memory module

      \param[in] x is the horizontal memory module coordinate

      \param[in] y is the vertical memory module coordinate
  */
  memory_base &memory_module(int x, int y) {
    geo::validate_x_y(x, y);
    return *memory_modules_bases[y][x];
  }


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


  /** Iterate on all the memory module bases of the AIE in an
      homogeneous way

      \param[in] F is the function to apply on each memory module base
  */
  template <typename F>
  void for_each_memory_base(F && f) {
    for (auto y = 0; y != geo::y_size; ++y)
      for (auto x = 0; x != geo::x_size; ++x)
        f(*memory_modules_bases[y][x]);
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


  /** Iterate on all the tile bases of the AIE in an homogeneous way

      \param[in] F is the function to apply on each tile base
  */
  template <typename F>
  void for_each_tile_base(F && f) {
    for (auto y = 0; y != geo::y_size; ++y)
      for (auto x = 0; x != geo::x_size; ++x)
        f(*tile_bases[y][x]);
  }

  XAieGbl_Tile tile_inst[geo::x_size][geo::y_size + 1];

  /// Create the AIE program with the tiles and memory modules
  program(AIEDevice &aie_d) : aie_d { aie_d } {
    XAieGbl_HwCfg aie_config;
    XAieGbl_Config *aie_config_ptr;
    XAieGbl aie_inst;

     // FIXME: hardcoded array offset = 0x800
    XAIEGBL_HWCFG_SET_CONFIG((&aie_config), geo::y_size, geo::x_size, 0x800);
    XAieGbl_HwInit(&aie_config);
    aie_config_ptr = XAieGbl_LookupConfig(XPAR_AIE_DEVICE_ID);
    (void)XAieGbl_CfgInitialize(&aie_inst, &tile_inst[0][0], aie_config_ptr);

    boost::hana::for_each(tiles, [&] (auto& t) {
        // Inform each tile about its program
        t.set_program(*this);
        // Inform each tile about their tile infrastructure
        t.set_tile_infrastructure(aie_d.tile(t.x, t.y));
        // Inform each tile about their hw tile inst. skip the first shim row.
        t.set_hw_tile(&tile_inst[t.x][t.y + 1]);

        // Keep track of each base tile
        tile_bases[t.y][t.x] = &t;
      });
  }


  /** Instantiate a kernel in a form that can be outlined by the SYCL
      device compiler

      \param[in] k is the kernel functor
  */

// Outliner for Method 1
// Both methods currently uses anonymous lambda name generation so the compiler
// will generate the name for it based on things like scope, typename, line
// number etc.
//
// To specify explicitly the name just change the template signature to
// template <typename KernelName, typename KernelType> and do not compile with
// the -fsycl-unnamed-lambda option.
#ifdef __SYCL_DEVICE_ONLY__
  template <typename KernelType>
  __attribute__((sycl_kernel))
  void kernel_outliner(KernelType k) {
    k.run();
  }
#endif

// Outliner for Method 2
//#ifdef __SYCL_DEVICE_ONLY__
//  template <typename KernelType>
//  __attribute__((sycl_kernel))
//  void kernel_outliner(KernelType k) {
//    k();
//  }
//#endif

  /** Launch the programs of all the tiles of the CGRA in their own
      CPU thread and wait for their completion.

      This is the main member function to use to launch the execution.
  */
  void run() {
    // Start each tile program in its own CPU thread
    boost::hana::for_each(tiles, [&] (auto& t) {
#ifdef __SYCL_DEVICE_ONLY__
        // Method 1: use it directly as a kernel wrapper
        // This still results in some "garbage" IR from the axi streams default
        // destructor, but you can run -O3 and it'll clean it up quite a bit
        // without nuking everything so it's progress. The result seems semi-
        // reasonable and passes through xchesscc at a reasonable speed
        kernel_outliner(std::move(t));

        // Method 2: This needs us to turn off diagnostic about std layout
        // but allows us to use some normal SYCL like lambda generation rather
        // treating it as a kernel function object.
        // This does result in rather different IR than the above method,
        // Method 1 results in a function wrapper that's closer to what is
        // generated by the original test cases run through cardano. Where it
        // takes some arguments to the variables used inside of it.
        // This expects the Tile as an argument, and accesses the data contained
        // inside. So there is different connotations/semantics to the programs
        // generated by each method.
//        auto kernel = [=, TileMove = std::move(t)]() mutable { TileMove.run(); };
//        kernel_outliner(kernel);
#else
        t.submit([&] {
             if (t.prerun())
              return;

            TRISYCL_DUMP_T("Starting ME tile (" << t.x << ',' << t.y
                           << ") linear id = " << t.linear_id());

            {
              t.load_elf("aie-increment.elf");
            }

            t.core_reset();
            t.core_run();
            t.core_wait();
            t.core_stop();

            t.postrun();

            TRISYCL_DUMP_T("Stopping ME tile (" << t.x << ',' << t.y << ')');
          })
#endif
    });

// TODO: look into if this ndef is required, mostly just trying to avoid scary
// thread code inside kernels.
#ifndef __SYCL_DEVICE_ONLY__
//     Wait for the end of the execution of each tile
    boost::hana::for_each(tiles, [&] (auto& t) {
        TRISYCL_DUMP_T("Joining AIE tile (" << t.x << ',' << t.y << ')');
        t.wait();
        TRISYCL_DUMP_T("Joined AIE tile (" << t.x << ',' << t.y << ')');
      });
#endif

    TRISYCL_DUMP_T("Total size of the own memory of all the tile programs: "
                   << std::dec << sizeof(tiles) << " bytes.");
    TRISYCL_DUMP_T("Total size of the memory modules: "
                   << std::dec << sizeof(memory_modules) << " bytes.");
  }


  /// Access the cascade connections
  auto &cascade() {
    return aie_d.cascade();
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
