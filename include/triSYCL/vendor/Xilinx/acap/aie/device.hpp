#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP

/** \file

    SYCL-friendly device view of AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <utility>

#include "device/detail/device.hpp"
#include "triSYCL/device/facade/device.hpp"

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie {

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/
template <typename Layout>
class device : public facade::device<device<Layout>, detail::device<Layout>> {
  /// The type encapsulating the implementation
  using dd = detail::device<Layout>;

  /// The façade used to implement part of the use facing type
  using facade_t = facade::device<device<Layout>, dd>;

  /// Make the implementation member directly accessible in this class
  using facade_t::implementation;

 public:
  /// Expose some useful internal implementation.
  /// Look at them for their documentation
  using geo = typename dd::geo;
  using cass = typename dd::cass;
  using csp = typename dd::csp;
  using cmp = typename dd::cmp;
  using sass = typename dd::sass;
  using ssp = typename dd::ssp;
  using smp = typename dd::smp;

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
  xaie::XAie_DevInst *get_dev_inst() { return implementation->get_dev_inst(); }
#endif

  /// The default constructor makes a new device
  device()
      : facade_t { std::make_shared<dd>() } {}

  /** Apply a function for each tile index of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F> void for_each_tile_index(F&& f) {
    implementation->for_each_tile_index(std::forward<F>(f));
  };

  /** Apply a function for each tile infrastructure of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F> void for_each_tile(F&& f) {
    implementation->for_each_tile(std::forward<F>(f));
  };

  /** Apply a function for each x tile index of the device

      \param f is a callable that is to be called like \c f(x) for
      each horizontal index value
  */
  template <typename F> void for_each_tile_x_index(F&& f) {
    implementation->for_each_tile_x_index(std::forward<F>(f));
  };

  /** Apply a function for each y tile index of the device

      \param f is a callable that is to be called like \c f(x) for
      each vertical index value
  */
  template <typename F> void for_each_tile_y_index(F&& f) {
    implementation->for_each_tile_y_index(std::forward<F>(f));
  };

  /** Access to the common infrastructure part of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto& tile(int x, int y) { return implementation->tile(x, y); }

  /** Access to the common infrastructure part of a tile memory

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto& mem(int x, int y) { return implementation->mem(x, y); }

  /** Access to the shim tile

      \param[in] x is the horizontal coordinate of the shim tile

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto& shim(int x) { return implementation->shim(x); }

  /// Access the cascade connections
  /// \todo To remove?
  auto& cascade() { return implementation->cascade(); }

  /// Create a queue on this device
  auto queue() { return vendor::xilinx::acap::aie::queue { *this }; }

  /** Create a program execution for this device

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename Device, int X, int Y>
            typename Tile = acap::aie::tile,
            template <typename Device, int X, int Y>
            typename Memory = acap::aie::memory>
  auto program() {
    return aie::program<device, Tile, Memory> { *this };
  }

  /** Shortcut to run synchronously a program on this queue

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename Device, int X, int Y>
            typename Tile = acap::aie::tile,
            template <typename Device, int X, int Y>
            typename Memory = acap::aie::memory>
  void run() {
    queue().template run<Tile, Memory>();
  }

  /** Shortcut to run synchronously an heterogeneous invocable on this
      queue

      \param f is an invocable taking an heterogeneous tile handler
  */
  template <typename Invocable> void run(Invocable&& f) {
    queue().run(f);
  }

  /** Shortcut to run synchronously a uniform invocable on
      this queue

      \param f is an invocable taking a uniform tile handler
  */
  template <typename Invocable> void uniform_run(Invocable&& f) {
    queue().uniform_run(f);
    queue().wait();
  }

  /** Connect the ports of 2 tiles or shims together with an
      hyperspace switched circuit, jumping over the underlying routing
      infrastructure.

      This is used to speed up communication in emulation and to skip
      the need of a routing planner.

      \todo To deprecate and implement in another way?

      \param[in] T is the type of the data to be transferred

      \param[in] SrcPort is the type of the source port, such as
      port::tile or port::shim

      \param[in] DstPort is the type of the destination port, such as
      port::tile or port::shim

      \throws trisycl::runtime_error if some coordinates or port
      numbers are invalid

      \todo Refactor, make the difference between user & physical ports
  */
  template <typename SrcPort, typename DstPort>
  void connect(SrcPort&& src, DstPort&& dst) {
    implementation->connect(std::forward<SrcPort>(src),
                            std::forward<DstPort>(dst));
  }

  /** Apply a invocable on all the AXI stream of the neighborhood of
      each tile */
  template <typename F> void for_each_tile_neighborhood(F&& f) {
    implementation->for_each_tile_neighborhood(std::forward<F>(f));
  }

  /** Display the device layout

      \param[in] file_name is the name of the file to write the LaTeX
      drawing into or use std::cerr by default
  */
  auto display(const std::string& file_name = {}) {
    return implementation->display(file_name);
  }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_HPP
