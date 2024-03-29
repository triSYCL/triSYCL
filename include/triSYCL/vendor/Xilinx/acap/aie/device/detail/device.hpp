#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_DETAIL_DEVICE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_DETAIL_DEVICE_HPP

/** \file

    SYCL-friendly device view of AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <string>
#include <type_traits>

#include "magic_enum.hpp"
#include <boost/format.hpp>
#include <boost/hana.hpp>

#include "../../geography.hpp"
#include "../../queue.hpp"
#include "../../shim_tile.hpp"
#include "../../accessor.hpp"
#include "../../rpc.hpp"

#ifdef __SYCL_XILINX_AIE__
#include "../../xaie_wrapper.hpp"
#include "triSYCL/detail/kernel_desc.hpp"
#else
#include "triSYCL/detail/fiber_pool.hpp"
#include "../../tile_infrastructure.hpp"
#endif

/// \ingroup aie
/// @{

namespace trisycl::vendor::xilinx::acap::aie::detail {

#ifdef __SYCL_XILINX_AIE__

/// Type used to configure and execute work on one specific tile on the host.
template <typename Geo> class tile_hw_config : public tile_hw_base_impl<Geo> {
  xaie::handle dev_handle;

  class handler {
    struct accessor_data {
      /// Pointer to the start of the host accessor's representation.
      const char* id;
      /// Pointer the accessors data.
      void* ptr;
      /// Size in bytes of the accessors data
      size_t size;
      /// Size in bytes of the accessors representation
      size_t sizeof_acc;
      /// Address on device at which the buffer was placed.
      hw::dev_ptr<void> dev_addr;
    };

    /// Collection of accessor that may be transfer to the device by the kernel.
    std::vector<accessor_data> accessors;
    xaie::handle dev_handle;
    public:
    handler() = default;
    handler(xaie::handle h) : dev_handle(h) {}

    /// Register a new accessor that may need to be transferred.
    template <typename Acc> void add_accessor(const Acc& acc) {
      accessors.emplace_back(
          accessor_data { reinterpret_cast<const char*>(&acc), &*acc.begin(),
                          acc.get_size(), sizeof(acc) });
    }

#ifndef __SYCL_DEVICE_ONLY__
    /// Write the lambda on the device such that the kernel can use it.
    template <typename KernelDesc, typename KernelLambda>
    void write_lambda(KernelLambda& L, uint32_t dev_addr, uint32_t heap_start) {
      TRISYCL_DUMP2("Lambda address = " << (void*)(std::uintptr_t)dev_addr, "memory");

      /// Write the lambda to memory, the accessors will get corrected later.
      dev_handle.store<KernelLambda, /*no_check*/true>(dev_addr, L);
      for (int i = 0; i < KernelDesc::getNumParams(); i++) {
        ::trisycl::detail::kernel_param_desc_t kdesc = KernelDesc::getParamDesc(i);
        /// Only accessor are supported and need special handling.
        if (kdesc.kind == ::trisycl::detail::kernel_param_kind_t::kind_accessor) {
          void* acc_addr = reinterpret_cast<char*>(&L) + kdesc.offset;
          bool was_found = false;
          for (auto& acc : accessors) {
            /// Check if the accessor in acc is the same as the one at address
            /// acc_addr In this function we do not have type information about
            /// the accessor so we rely on accessor memory represention to
            /// identify which accessor is where in the lambda capture.
            if (std::memcmp(acc_addr, acc.id, acc.sizeof_acc) == 0) {
              assert(!was_found && "accessor was found twice ??");
              was_found = true;
              ::trisycl::vendor::xilinx::acap::aie::device_accessor_base dev_acc;
              /// Allocate space on device for the accessors's data.
              uint32_t buff_addr =
                  acap::heap::malloc(dev_handle, heap_start, acc.size);
              /// Copy the accessor's data to the device.
              dev_handle.memcpy_h2d(buff_addr, acc.ptr, acc.size);
              dev_acc.size = acc.size;
              /// Calculated the address from the device's perspective
              dev_acc.ptr = hw::dev_ptr<void>::create(dev_handle.get_self_dir(),
                                                      buff_addr);
              /// Overwrite the host accessor with the device accessor.
              dev_handle.store(dev_addr + kdesc.offset, dev_acc);
              /// Store device address of the accessor such that we can do the write_back.
              acc.dev_addr = dev_acc.ptr;
            }
          }
        }
      }
    }

    /// Write all accessors that were written to device back to the host
    void write_back() {
      for (auto e : accessors)
        if (e.dev_addr)
          dev_handle.memcpy_d2h(e.ptr, e.dev_addr.get_offset(), e.size);
    }
#endif
    template <typename T> void single_task(T&& func) {
      detail::exec_kernel<tile_hw_base_impl<Geo>>{}.exec(
          dev_handle, std::forward<T>(func), 0, this);
    }
  } cgh;

public:
  tile_hw_config() = default;
  tile_hw_config(xaie::handle h) : dev_handle(h), cgh(dev_handle) {}

  template <typename T> void submit(T func) { std::forward<T>(func)(cgh); }

  template <typename T> void single_task(T&& func) {
    cgh.single_task(std::forward<T>(func));
  }

  void write_back() { cgh.write_back(); }
  /// Configure a connection of the core tile AXI stream switch

  auto &connect(typename Geo::core_axi_stream_switch::slave_port_layout sp,
                typename Geo::core_axi_stream_switch::master_port_layout mp) {
#if defined(__SYCL_DEVICE_ONLY__)
    assert(false && "should never be executed on device");
#endif
    auto slave_port_type = xaie::acap_port_to_xaie_port_type</*is_slave*/true>(sp);
    auto slave_port_id = xaie::acap_port_to_xaie_port_id(sp, slave_port_type);
    auto master_port_type = xaie::acap_port_to_xaie_port_type</*is_slave*/false>(mp);
    auto master_port_id = xaie::acap_port_to_xaie_port_id(mp, master_port_type);
    dev_handle.stream_connect(slave_port_type, slave_port_id, master_port_type,
                              master_port_id);
    return *this;
  }
};
#endif

/** Create a SYCL-like device view of an AI Engine CGRA with some layout

    \param Layout is the layout description of the machine to
    instantiate with the physical size
*/
template <typename Layout> struct device {

  /// The geography of the CGRA
  using geo = geography<Layout>;

  /// Naming shortcut for the core AXI stream switch
  using cass = typename geo::core_axi_stream_switch;

  /// Naming shortcut for the slave ports of the core AXI stream switch
  using csp = typename cass::slave_port_layout;

  /// Naming shortcut for the master ports of the core AXI stream switch
  using cmp = typename cass::master_port_layout;

  /// Naming shortcut for the shim AXI stream switch
  using sass = typename geo::shim_axi_stream_switch;

  /// Naming shortcut for the slave ports of the shim AXI stream switch
  using ssp = typename sass::slave_port_layout;

  /// Naming shortcut for the master ports of the shim AXI stream switch
  using smp = typename sass::master_port_layout;

  /** A fiber pool executor to run the infrastructure powered by
      TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS std::thread */
  ::trisycl::detail::fiber_pool fiber_executor {
    1, ::trisycl::detail::fiber_pool::sched::round_robin, false
  };
  /* { TRISYCL_XILINX_AIE_FIBER_EXECUTOR_THREADS,
       detail::fiber_pool::sched::work_stealing, false };
  */

#ifndef __SYCL_XILINX_AIE__
  /// Keep track of all the (non detail) infrastructure tiles of this device
  aie::tile_infrastructure<geo> ti[geo::y_size][geo::x_size];

  /** Access to the common infrastructure part of a tile

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto& tile(int x, int y) {
    geo::validate_x_y(x, y);
    return ti[y][x];
  }
#else
  tile_hw_config<geo> thc[geo::y_size][geo::x_size];

  auto& tile(int x, int y) { return thc[x][y]; }
#endif

  /** Apply a function for each tile index of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F> void for_each_tile_index(F&& f) {
    for (auto [x, y] :
         ranges::views::cartesian_product(ranges::views::iota(0, geo::x_size),
                                          ranges::views::iota(0, geo::y_size)))
      f(x, y);
  };

  /** Apply a function for each tile infrastructure of the device

      \param f is a callable that is to be called like \c f(x,y) for
      each tile
  */
  template <typename F> void for_each_tile(F&& f) {
    for_each_tile_index([&](auto x, auto y) { f(tile(x, y)); });
  };

  /** Apply a function for each x tile index of the device

      \param f is a callable that is to be called like \c f(x) for
      each horizontal index value
  */
  template <typename F> void for_each_tile_x_index(F&& f) {
    for (auto x : ranges::views::iota(0, geo::x_size))
      f(x);
  };

  /** Apply a function for each y tile index of the device

      \param f is a callable that is to be called like \c f(x) for
      each vertical index value
  */
  template <typename F> void for_each_tile_y_index(F&& f) {
    for (auto y : ranges::views::iota(0, geo::y_size))
      f(y);
  };

#ifndef __SYCL_XILINX_AIE__
  /** Shortcut to access to the common infrastructure part of a tile memory

      \param[in] x is the horizontal tile coordinate

      \param[in] y is the vertical tile coordinate

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto& mem(int x, int y) {
    return tile(x, y).mem();
  }
#endif

  /** The shim tiles on the lower row of the tile array

      For now we consider only homogeneous shim tiles.
  */
  shim_tile<device> shims[geo::x_size];

  /** Access to the shim tile

      \param[in] x is the horizontal coordinate of the shim tile

      \throws trisycl::runtime_error if the coordinate is invalid
  */
  auto& shim(int x) {
    geo::validate_x(x);
    return shims[x];
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
    static_assert(std::is_same_v<SrcPort, port::tile> ||
                      std::is_same_v<SrcPort, port::shim>,
                  "SrcPort type should be port::tile or port::shim");
    static_assert(std::is_same_v<DstPort, port::tile> ||
                      std::is_same_v<DstPort, port::shim>,
                  "DstPort type should be port::tile or port::shim");
    // Get the destination in the final switch
    auto channel = [&] {
      if constexpr (std::is_same_v<DstPort, port::tile>)
        return tile(dst.x, dst.y).in_connection(dst.port);
      else if constexpr (std::is_same_v<DstPort, port::shim>)
        return shim(dst.x).bli_in_connection(dst.port);
    }();
    // And connect the source  port to it
    if constexpr (std::is_same_v<SrcPort, port::tile>)
      tile(src.x, src.y).out_connection(src.port) = channel;
    else if constexpr (std::is_same_v<SrcPort, port::shim>)
      shim(src.x).bli_out_connection(src.port) = channel;
  }

  /// Apply a function on all the AXI stream of the neighborhood of each tile
  template <typename F> void for_each_tile_neighborhood(F&& f) {
    for_each_tile_index([&](auto x, auto y) {
      // No CTAD yet with Boost::Hana and Clang++-10 (but works with g++-9)
      boost::hana::for_each(
          geo::core_axi_stream_switch::interconnect, [&](auto connections) {
            auto [dx, dy, master_range, slave_range] = connections;
            if (geo::is_x_y_valid(x + dx, y + dy))
              for (auto [o, i] : ranges::views::zip(master_range, slave_range))
                f(x, y, x + dx, y + dy, o, i);
          });
    });
  }

#if defined(__SYCL_XILINX_AIE__)
  /// The host needs to set up the device when executing on real device
  /// The following are macro that declare variables in our case member
  /// variables. the xaie:: before them is because there type is in the
  /// namespace xaie::.
  /// this declares aie_config of type xaie::XAie_Config.
  xaie::XAie_SetupConfig(aie_config, aiev1::dev_gen, aiev1::base_addr,
                         aiev1::col_shift, aiev1::row_shift, aiev1::num_hw_col,
                         aiev1::num_hw_row, aiev1::num_shim_row,
                         aiev1::mem_tile_row_start, aiev1::mem_tile_row_num,
                         aiev1::aie_tile_row_start, aiev1::aie_tile_row_num);
  /// This declares \c aie_inst of type \c xaie::XAie_InstDeclare
  xaie::XAie_InstDeclare(aie_inst, &aie_config);

  /// Returns the pointer to libXAiengine's device.
  xaie::XAie_DevInst* get_dev_inst() {
    return &aie_inst;
  }
#endif

  /// Build some device level infrastructure
  device() {
    // Initialization of the AI Engine tile constructs from LibXAiengine
#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
    // For host side when executing on ACAP hardware.
    /// Initialize the device
    TRISYCL_XAIE(xaie::XAie_CfgInitialize(&aie_inst, &aie_config));
    /// Request access to all tiles.
    TRISYCL_XAIE(xaie::XAie_PmRequestTiles(&aie_inst, NULL, 0));
    // Initialize all the tiles with their network connections first
#endif
    for_each_tile_index([&](auto x, auto y) {
#if !defined(__SYCL_XILINX_AIE__)
      // Create & start the tile infrastructure.
      // For CPU emulation
      tile(x, y) = {x, y, fiber_executor};
#else
      tile(x, y) = xaie::handle{xaie::acap_pos_to_xaie_pos({x, y}), get_dev_inst()};
#endif
    });

#if !defined(__SYCL_XILINX_AIE__)
    // TODO: this should be enabled on hardware when it is working but for now
    // it isn't

    for_each_tile_x_index([&](auto x) {
      // Start the shim infrastructure
      shim(x).start(x, fiber_executor);
    });

    // Only then we can connect the inter-core tile AXI stream NoC
    for_each_tile_neighborhood(
        [&](auto x, auto y, auto nx, auto ny, auto m, auto s) {
          tile(x, y).output(m) = tile(nx, ny).input(s);
        });

    /// Make the connections around the shim tiles
    for_each_tile_x_index([&](auto x) {
      // The connection from shim tile to core tile
      for (auto [o, i] :
           ranges::views::zip(sass::m_north_range, cass::s_south_range))
        shim(x).output(o) = tile(x, 0).input(i);
      // The connection from core tile to shim tile
      for (auto [o, i] :
           ranges::views::zip(cass::m_south_range, sass::s_north_range))
        tile(x, 0).output(o) = shim(x).input(i);
      if (!geo::is_east_column(x))
        for (auto [o, i] :
             ranges::views::zip(sass::m_east_range, sass::s_west_range))
          shim(x).output(o) = shim(x + 1).input(i);
      if (!geo::is_west_column(x))
        for (auto [o, i] :
             ranges::views::zip(sass::m_west_range, sass::s_east_range))
          shim(x).output(o) = shim(x - 1).input(i);
    });
#endif
  }

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
  // For host side when executing on acap hardware
  ~device() {
    /// close the device instant.
    TRISYCL_XAIE(xaie::XAie_Finish(&aie_inst));
  }
#endif

#ifndef __SYCL_XILINX_AIE__
  /** Display the device layout

      \param[in] file_name is the name of the file to write the LaTeX
      drawing into or use std::cerr by default
  */
  auto display(const std::string& file_name = {}) {
    // Compute the drawing size starting with the individual tile size
    auto tile_size = tile(0, 0).display_size();
    // And expanding according to the device size. Add 1 in each
    // dimension to fit in the page because of the borders
    auto x_size = tile_size.x() * geo::x_size + 1;
    auto y_size = tile_size.y() * geo::y_size + 1;
    /// The LaTeX generation is handled by a LaTeX context
    latex::context c { { x_size, y_size }, file_name };

    for_each_tile_index([&](auto x, auto y) { tile(x, y).display(c); });

    // Connect each tile to its neighbors
    for_each_tile_neighborhood(
        [&](auto x, auto y, auto nx, auto ny, auto m, auto s) {
          c.add((boost::format { R"(
    \draw (node cs:name=TileX%1%Y%2%M%3%)
       -- (node cs:name=TileX%4%Y%5%S%6%);)" } %
                 x % y % c.clean_node(magic_enum::enum_name(m)) % nx % ny %
                 c.clean_node(magic_enum::enum_name(s)))
                    .str());
        });

    c.display();
  }
#endif
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie::detail

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DEVICE_DETAIL_DEVICE_HPP
