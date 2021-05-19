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
#include "xaie_wrapper.hpp"
#include "triSYCL/detail/program_manager.hpp"
#include "triSYCL/detail/kernel_desc.hpp"

/// TODO: Perhaps worth pushing all Lib X AI Engine functionallity we use down
/// into a C++ API so it can all be excluded with one #ifdef and kept nice and
/// cleanly

/// \ingroup acap
///  @{

/** \defgroup aie AI Engine CGRA

    Extensions to support explicit AI Engine system-wide programming in C++
    @{
*/

namespace trisycl::vendor::xilinx::acap::aie {

/// This could be extracted into a macro to make it generic.
template <typename T> class has_function_lock {
  template <typename...> static constexpr bool true_t = true;
  static std::false_type detect(...) { return {}; }
  template <typename Ty,
            typename std::enable_if<
                true_t<decltype(std::declval<Ty>().lock())>, int>::type = 0>
  static std::true_type detect(Ty) {
    return {};
  }

public:
  static constexpr bool value = decltype(detect(std::declval<T>()))::value;
};


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

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
  // for host side on device execution
  xaie::XAie_SetupConfig(aie_config, aiev1::dev_gen, aiev1::base_addr,
                         aiev1::col_shift, aiev1::row_shift, aiev1::num_hw_col,
                         aiev1::num_hw_row, aiev1::num_shim_row,
                         aiev1::mem_tile_row_start, aiev1::mem_tile_row_num,
                         aiev1::aie_tile_row_start, aiev1::aie_tile_row_num);
  xaie::XAie_InstDeclare(aie_inst, &aie_config);
#endif

  /// Create the AIE program with the tiles and memory modules
  program(AIEDevice &aie_d) : aie_d{aie_d} {
    // Initialization of the AI Engine tile constructs from Lib X AI Engine
#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
    // for host side on device execution
    TRISYCL_XAIE(xaie::XAie_CfgInitialize(&aie_inst, &aie_config));
    TRISYCL_XAIE(xaie::XAie_PmRequestTiles(&aie_inst, NULL, 0));
#endif

    boost::hana::for_each(tiles, [&](auto &t) {
      // Inform each tile about its program
    t.set_program(*this);
#ifndef __SYCL_DEVICE_ONLY__
    // always execpt for device side of hardware execution

    // Inform each tile about their tile infrastructure
    t.set_tile_infrastructure(aie_d.tile(t.x, t.y));
#endif
#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
      // for host side on device execution

    // Inform each tile about their hw tile instance. Skip the first shim row.
    t.set_dev_handle(
        xaie::handle{xaie::acap_pos_to_xaie_pos({t.x, t.y}), &aie_inst});
#endif
    // Keep track of each base tile
    tile_bases[t.y][t.x] = &t;
    });
    // Connect each memory to its infrastructure
    boost::hana::for_each(memory_modules, [&](auto &m) {
      // Inform each tile about their tile infrastructure
      m.set_memory_infrastructure(aie_d.mem(m.x, m.y));
      // Keep track of each base tile
      memory_modules_bases[m.y][m.x] = &m;
    });
  }

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
  // for host side on device execution
  ~program() {
    TRISYCL_XAIE(xaie::XAie_Finish(&aie_inst));
  }
#endif


  // Outliner for Method 2
  //#ifdef __SYCL_DEVICE_ONLY__
  //  template <typename KernelType>
  //  __attribute__((sycl_kernel))
  //  void kernel_invoker(KernelType k) {
  //    k();
  //  }
  //#endif

  /// Wait for the end of the execution of each tile
  void wait() {
    /// There the device doesn't need to wait for itself to execute.
#if !defined(__SYCL_DEVICE_ONLY__)
    boost::hana::for_each(tiles, [&](auto &t) {
      TRISYCL_DUMP2("Joining AIE tile (" << t.x << ',' << t.y << ')', "exec");
      t.get_dev_handle().core_wait();
      TRISYCL_DUMP2("Joined AIE tile (" << t.x << ',' << t.y << ')', "exec");
      t.postrun();
    });
#endif
  }

  void lock() {
    boost::hana::for_each(tiles, [&](auto &t) {
      if constexpr (has_function_lock<decltype(t)>::value)
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
  template <typename Invocable>
  void run(Invocable&& f) {
    lock();
    // Start each tile program in its own executor
    boost::hana::for_each(tiles, [&] (auto& t) {
        t.single_task([&] {
            TRISYCL_DUMP_T("Starting AIE tile (" << t.x << ',' << t.y
                           << ") linear id = " << t.linear_id());
            /// Each tile gets its own copy of work
            auto kernel = [&, work = f] { work(t); };
            using kernel_type = decltype(kernel);
            // Use the kernel type as its SYCL name too
            kernel_invoker<kernel_type, kernel_type>(kernel);
            TRISYCL_DUMP_T("Stopping AIE tile (" << t.x << ',' << t.y << ')');
          });
      });
    wait();
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
