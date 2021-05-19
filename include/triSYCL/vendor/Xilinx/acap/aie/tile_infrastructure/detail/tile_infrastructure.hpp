#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_DETAIL_TILE_INFRASTRUCTURE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_DETAIL_TILE_INFRASTRUCTURE_HPP

/** \file

    The basic AI Engine homogeneous tile, with the common
    infrastructure to all the tiles, i.e. independent of x & y
    coordinates, but also from the tile program itself.

    This tile can be seen as the raw CGRA subdevice to run elemental
    functions.

    This is owned by the device, so for example the AXI stream switch
    configuration and packet can survive to some program changes.

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <future>
#include <memory>
#include <optional>

#include "magic_enum.hpp"
#include <boost/format.hpp>
#include <range/v3/all.hpp>

#include "../../axi_stream_switch.hpp"
#include "../../dma.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/detail/ranges.hpp"
#include "triSYCL/vendor/Xilinx/config.hpp"
#include "triSYCL/vendor/Xilinx/latex.hpp"

namespace trisycl::vendor::xilinx::acap::aie::detail {

/// \ingroup aie
/// @{

/** The AI Engine tile infrastructure common to all the tiles

    This allows some type erasure while accessing the common
    tile infrastructure.

    \param AIE is the type representing the full CGRA with the
    programs and memory contents
*/
template <typename Geography> class tile_infrastructure {

 public:
  using geo = Geography;
  using axi_ss_geo = typename geo::core_axi_stream_switch;
  using mpl = typename axi_ss_geo::master_port_layout;
  using spl = typename axi_ss_geo::slave_port_layout;
  using axi_ss_t = axi_stream_switch<axi_ss_geo>;

 private:
  /// Keep the horizontal coordinate
  int x_coordinate;

  /// Keep the vertical coordinate
  int y_coordinate;

  /// The AXI stream switch of the core tile
  axi_ss_t axi_ss;

  /** Sending DMAs

      Use std::optional to postpone initialization */
  std::array<std::optional<sending_dma<axi_ss_t>>, axi_ss_geo::s_dma_size>
      tx_dmas;

#if defined(__SYCL_XILINX_AIE__)
#if !defined(__SYCL_DEVICE_ONLY__)
  xaie::handle dev_handle;
#endif
#else
#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
  /// Keep track of the fiber executor
  ::trisycl::detail::fiber_pool* fe;

  /// To shepherd the working fiber
  ::trisycl::detail::fiber_pool::future<void> future_work;
#else
  /// Keep track of the std::thread execution in this tile
  std::future<void> future_work;
#endif
#endif

#ifdef __SYCL_DEVICE_ONLY__
  /// On device trigger outlining.

  template <typename KernelName, typename KernelType>
  __attribute__((sycl_kernel))
  __attribute__((annotate("xilinx_acap_tile", KernelType::x,
                          KernelType::y))) void
  kernel_invoker_run(KernelType &k) {
    k.run();
  }
  template <typename KernelName, typename KernelType>
  __attribute__((sycl_kernel))
  __attribute__((annotate("xilinx_acap_tile", KernelType::x,
                          KernelType::y))) void
  kernel_invoker_call(KernelType &k) {
    k();
  }
#else
  /// This isn't called on the host unless we are in CPU emulation so we invoke
  /// the kernel function.

  template <typename KernelName, typename KernelType>
  void kernel_invoker_run(KernelType &k) {
    k.run();
  }
  template <typename KernelName, typename KernelType>
  void kernel_invoker_call(KernelType &k) {
    k();
  }
#endif

  template <
      typename KernelName, typename KernelType,
      std::enable_if_t<std::is_invocable_r<void, KernelType>::value, int> = 0>
  void kernel_invoker(KernelType &k) {
    kernel_invoker_call<KernelName, KernelType>(k);
  }
  template <
      typename KernelName, typename KernelType,
      std::enable_if_t<!std::is_invocable_r<void, KernelType>::value, int> = 0>
  void kernel_invoker(KernelType &k) {
    kernel_invoker_run<KernelName, KernelType>(k);
  }

  /** Map the user input port number to the AXI stream switch port

      \param[in] port is the user port to use
  */
  static auto translate_input_port(int port) {
    return axi_ss_t::translate_port(port, spl::me_0, spl::me_last,
                                    "The core input port is out of range");
  }

  /** Map the user output port number to the AXI stream switch port

      \param[in] port is the user port to use
  */
  static auto translate_output_port(int port) {
    return axi_ss_t::translate_port(port, mpl::me_0, mpl::me_last,
                                    "The core output port is out of range");
  }

 public:

#ifdef __SYCL_DEVICE_ONLY__
tile_infrastructure() = default;
#endif

  /** Start the tile infrastructure associated to the AIE device

      \param[in] x is the horizontal coordinate for this tile

      \param[in] y is the vertical coordinate for this tile

      \param[in] fiber_executor is the executor used to run
      infrastructure details
  */
  tile_infrastructure(int x, int y,
                      ::trisycl::detail::fiber_pool& fiber_executor)
      : x_coordinate { x }
      , y_coordinate { y }
#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
      , fe { &fiber_executor }
#endif
  {
#if !defined(__SYCL_XILINX_AIE__)
    // TODO: this should be enabled on hardware when it is working but for now
    // it isn't

    // Connect the core receivers to its AXI stream switch
    for (auto p : views::enum_type(mpl::me_0, mpl::me_last))
      output(p) =
          std::make_shared<port_receiver<axi_ss_t>>(axi_ss, "core_receiver");
    axi_ss.start(x, y, fiber_executor);
    /* Create the core tile receiver DMAs and make them directly the
       switch output ports */
    for (auto p : axi_ss_geo::m_dma_range)
      output(p) =
          std::make_shared<receiving_dma<axi_ss_t>>(axi_ss, fiber_executor);
    /* Create the core tile sender DMAs and connect them internally to
       their switch input ports */
    for (const auto& [d, p] :
         ranges::views::zip(tx_dmas, axi_ss_geo::s_dma_range))
      d.emplace(fiber_executor, input(p));
#endif
  }

#if defined(__SYCL_XILINX_AIE__) && !defined(__SYCL_DEVICE_ONLY__)
  // for host side on device execution
  /// Store a way to access to hw tile instance
  void set_dev_handle(xaie::handle h) {
    dev_handle = h;
  }
  xaie::handle get_dev_handle() const {
    return dev_handle;
  }
#endif

  /// Get the horizontal coordinate
  int x() { return x_coordinate; }

  /// Get the vertical coordinate
  int y() { return y_coordinate; }

  /** Get the user input connection from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in_connection(int port) {
    /* The input port for the core is actually the corresponding
       output on the switch */
    return axi_ss.out_connection(translate_output_port(port));
  }

  /** Get the user output connection to the AXI stream switch

      \param[in] port is port to use
  */
  auto& out_connection(int port) {
    /* The output port for the core is actually the corresponding
       input on the switch */
    return axi_ss.in_connection(translate_input_port(port));
  }

  /** Get the user input port from the AXI stream switch

      \param[in] port is the port to use
  */
  auto& in(int port) {
    TRISYCL_DUMP_T("in(" << port << ") on tile(" << x_coordinate << ','
                         << y_coordinate << ')');
    return *in_connection(port);
  }

  /** Get the user output port to the AXI stream switch

      \param[in] port is the port to use
  */
  auto& out(int port) {
    TRISYCL_DUMP_T("out(" << port << ") on tile(" << x_coordinate << ','
                          << y_coordinate << ')');
    return *out_connection(port);
  }

  /** Get access to a receiver DMA

      \param[in] id specifies which DMA to access */
  auto& rx_dma(int id) {
    /** The output of the switch is actually a receiving DMA, so we
        can view it as a DMA */
    return static_cast<receiving_dma<axi_ss_t>&>(*output(
        axi_ss_t::translate_port(id, mpl::dma_0, mpl::dma_last,
                                 "The receiver DMA port is out of range")));
  }

  /** Get access to a transmit DMA

      \param[in] id specifies which DMA to access */
  auto& tx_dma(int id) { return *tx_dmas.at(id); }

  /** Get the input router port of the AXI stream switch

      \param p is the slave_port_layout for the stream
  */
  auto& input(spl p) {
    // No index validation required because of type safety
    return axi_ss.input(p);
  }

  /** Get the output router port of the AXI stream switch

      \param p is the master_port_layout for the stream
  */
  auto& output(mpl p) {
    // No index validation required because of type safety
    return axi_ss.output(p);
  }

  /// Launch an invocable on this tile
  template <typename Work> void single_task(Work &&f) {
#ifdef __SYCL_XILINX_AIE__
#ifdef __SYCL_DEVICE_ONLY__
    // The outlining of the device binary Method 1: use it directly as a kernel
    // wrapper This still results in some "garbage" IR from the axi streams
    // default destructor, but you can run -O3 and it'll clean it up quite a bit
    // without nuking everything so it's progress. The result seems semi-
    // reasonable and passes through xchesscc at a reasonable speed
    kernel_invoker<typename std::decay<decltype(f)>::type>(f);

#else
    /// Host side

    // The name is captured by it's non-reference type and has to be in
    // the cl::sycl::detail namespace as the integration header is
    // defined to be in this namespace (and all our implementation
    // resides in trisycl by default, so ::detail resolves to
    // trisycl::detail)
    auto kernelName = ::trisycl::detail::KernelInfo<
        typename std::decay<decltype(f)>::type>::getName();

    auto kernelImage =
        ::trisycl::detail::program_manager::instance()->get_image(kernelName);

#ifdef TRISYCL_DEBUG_IMAGE
    // Image Dump using name retrieval for Debug, separate debug define
    // as dumping 400 images when at maximum array capacity is not
    // necessarily something you always want to do when debugging.
    //
    // This differentiates from the program_manager image dump in that
    // it helps check whether the names are correctly correlating to the
    // correct elf images and if there is some breakage in the storing
    // of the images.
    detail::program_manager::instance()->image_dump(
        kernelName, "run_aie_" + kernelName + ".elf");
#endif
    {
      // auto Transaction = f.get_transaction();
      f.get_dev_handle().core_reset();

      TRISYCL_DUMP2("Loading Kernel " << kernelName << " ELF to tile (" << f.x
                                      << ',' << f.y
                                      << ") linear id = " << f.linear_id(),
                    "exec");

      f.get_dev_handle().load_elf_image(kernelImage);

      TRISYCL_DUMP2("Loaded Kernel " << kernelName << " ELF to tile (" << f.x
                                     << ',' << f.y
                                     << ") linear id = " << f.linear_id()
                                     << "beginning tile execution",
                    "exec");
      /// Setup DMA for parameter passing
      f.get_dev_handle().mem_dma(hw_mem::args_beg_off, hw_mem::args_size);
      if (!f.prerun())
        return;

      TRISYCL_DUMP2("Starting AIE tile ("
                        << f.x << ',' << f.y
                        << ") linear id = " << f.linear_id() << ","
                        << "Associated Tile Kernel Name: " << kernelName
                        << "- beginning prerun execution",
                    "exec");

      f.get_dev_handle().core_run();
    }
#endif
#else
    if (future_work.valid())
      throw std::logic_error("Something is already running on this tile!");
    // Launch the tile program immediately on a new executor engine
    /** \todo In a device implementation we should have a real
        tile_handler type making sense on the device instead of just
        *this */
    auto kernel = [&] {
      if constexpr (requires { f(); })
        /* If the invocable is not interested by the handler, do not
           provide it. Add the outer lambda to avoid a warning about
           capturing this when non using it */
        return [work = std::forward<Work>(f)] { return work(); };
      else
        return [this, work = std::forward<Work>(f)] { return work(*this); };
    }();
#if TRISYCL_XILINX_AIE_TILE_CODE_ON_FIBER
    future_work = fe->submit(kernel);
#else
    future_work = std::async(std::launch::async, kernel);
#endif
#endif
  }

  /// Wait for the execution of the callable on this tile
  void wait() {
#ifdef __SYCL_XILINX_AIE__
#ifdef __SYCL_DEVICE_ONLY__
/// Device side

/// Noop

#else
/// Host side

/// TODO

#endif
#else
    if (future_work.valid())
      future_work.get();
#endif
  }

  /// Configure a connection of the core tile AXI stream switch
  void connect(typename geo::core_axi_stream_switch::slave_port_layout sp,
               typename geo::core_axi_stream_switch::master_port_layout mp) {
    axi_ss.connect(sp, mp);
  }

  /// Compute the size of the graphics representation of the processor
  static vec<int, 2> display_core_size() {
    // This is the minimum rectangle fitting all the processor outputs & inputs
    return { 1 + ranges::distance(axi_ss_geo::m_me_range),
             1 + ranges::distance(axi_ss_geo::s_me_range) };
  }

  /// Compute the size of the graphics representation of the tile
  static vec<int, 2> display_size() {
    // Just the sum of the size of its content
    return display_core_size() + axi_ss_t::display_size();
  }

  /// Display the tile to a LaTeX context
  void display(latex::context& c) const {
    auto get_tikz_coordinate = [&](auto x, auto y) {
      auto const [x_size, y_size] = display_size();
      return (boost::format { "(%1%,%2%)" }
              // Scale real LaTeX coordinate to fit
              % c.scale(x_coordinate * x_size + x) %
              c.scale(y_coordinate * y_size + y))
          .str();
    };
    c.add((boost::format { "  \\begin{scope}[name prefix = TileX%1%Y%2%]" } %
           x_coordinate % y_coordinate)
              .str());
    axi_ss.display(c, display_core_size(), get_tikz_coordinate);

    // Connect the core receivers to its AXI stream switch
    for (auto [i, p] : axi_ss_geo::m_me_range | ranges::views::enumerate) {
      c.add(
          (boost::format { R"(
    \coordinate(CoreIn%1%) at %2%;
    \node[rotate=90,anchor=east](CoreIn%1%Label) at %2% {in(%1%)};
    \draw (node cs:name=MMe%1%)
       -| (node cs:name=CoreIn%1%);)" } %
           i %
           get_tikz_coordinate(i, ranges::distance(axi_ss_geo::m_me_range) + 1))
              .str());
    };
    // Connect the core senders to its AXI stream switch
    for (auto [i, p] : axi_ss_geo::s_me_range | ranges::views::enumerate) {
      c.add(
          (boost::format { R"(
    \coordinate(CoreOut%1%) at %2%;
    \node[anchor=east](CoreOut%1%Label) at %2%  {out(%1%)};
    \draw (node cs:name=CoreOut%1%)
       -| (node cs:name=SMe%1%);)" } %
           i %
           get_tikz_coordinate(ranges::distance(axi_ss_geo::s_me_range), i + 1))
              .str());
    };
    c.add((boost::format { R"(
    \node[black] () at %1% {\texttt{tile<%2%,%3%>}};
    \begin{scope}[on background layer]
      \node [fill=orange!30, fit={(node cs:name=CoreIn0Label)
                                  (node cs:name=CoreOut0Label)}]
            (Core) {};
    \end{scope}
  \end{scope}

)" } % get_tikz_coordinate(1, 0) %
           x_coordinate % y_coordinate)
              .str());
  }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie::detail

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_TILE_INFRASTRUCTURE_DETAIL_TILE_INFRASTRUCTURE_HPP