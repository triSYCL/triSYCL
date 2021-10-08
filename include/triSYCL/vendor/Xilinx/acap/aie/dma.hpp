#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DMA_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DMA_HPP

/** \file

    Model of a tile DMA

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "magic_enum.hpp"
#include <boost/fiber/buffered_channel.hpp>
#include <boost/format.hpp>

#include <range/v3/all.hpp>

#include "connection.hpp"
#include "triSYCL/detail/enum.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/vendor/Xilinx/latex.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** Implement a small DSL to chain DMA & locking operations like
    allowed by the hardware

    \param[in] Tile is the type of tile to keep a reference on it

    \param[in] DMA is the type of the DMA involved in the operation

    \param[in] LockRef is the type of the reference to the lock once a
    lock() operation is involved. Otherwise just use a \c bool as a
    dummy type internally.
*/
template <typename Tile, typename DMA, typename LockRef = bool> class dma_dsl {
  /// Keep track of the tile involved in the operation chain
  Tile& tile;

  /// Keep track of the DMA involved in the operation chain
  DMA& dma;

  /** Keep track of the current lock involved in the operation chain
      if any */
  LockRef lck;

 public:
  /// Keep track of operation with only DMA involved
  dma_dsl(Tile& t, DMA& d)
      : tile { t }
      , dma { d } {}

  /// Keep track of operation with DMA and lock involved
  dma_dsl(Tile& t, DMA& d, LockRef l)
      : tile { t }
      , dma { d }
      , lck { l } {}

  /// Select a lock for the following operations
  auto lock(int l) {
    auto& lock_ref = tile.mem().lock(l);
    return dma_dsl<Tile, DMA, decltype(lock_ref)> { tile, dma, lock_ref };
  }

  /// Enqueue a DMA receive command
  template <typename Cmd> auto& receive(Cmd&& cmd) {
    dma.receive(std::forward<Cmd>(cmd));
    // To be able to chain another DMA operation on it
    return *this;
  }

  /// Enqueue a DMA transfer command
  template <typename Cmd> auto& send(Cmd&& cmd) {
    dma.send(std::forward<Cmd>(cmd));
    // To be able to chain another DMA operation on it
    return *this;
  }

  /** Wait for the transfers to complete

      The API assumes that the program is written in such a way that
      at some point the queue will be empty so there is no continuous
      DMA command enqueuing for example. */
  auto& wait() {
    dma.wait();
    // To be able to chain another DMA operation on it
    return *this;
  }
};

/// Generic DMA
template <typename AXIStreamSwitch, typename SpecializedDMA>
class dma : ::trisycl::detail::debug<SpecializedDMA> {

  /// Use std::span for 1D contiguous transfers only for now
  // using dma_command = std::span<axi_packet::value_type>;

 private:
  /// The type of commands executable by the DMA. Very generic for now
  using dma_command = std::function<void(void)>;

  /// Queue for generic DMA commands
  boost::fibers::buffered_channel<dma_command> c { 8 };

  /// To shepherd the fiber playing the data mover behind the DMA
  ::trisycl::detail::fiber_pool::future<void> data_mover;

  /// Count the in-flight commands
  std::atomic<int> nb_command { 0 };

  /// To wait for the DMA command queue to drain
  boost::fibers::condition_variable waiting_room;

  /// To protect the waiting mechanism
  boost::fibers::mutex waiting_mutex;

  /// Wait for the fiber to complete
  void join() {
    // Get the value of the future, to get an exception if any
    data_mover.get();
  }

  /// Launch the data mover on the execution pool
  template <typename Invocable>
  void launch(::trisycl::detail::fiber_pool& fiber_executor, Invocable&& f) {}

  /// Dequeue a DMA command
  auto pop_command(auto& command) { return c.pop(command); }

  /// Notify a command has been processed
  void commit_command() {
    if (--nb_command == 0)
      // Warn anyone waiting for the DMA queue to drain it might be empty now
      waiting_room.notify_all();
  }

 protected:
  /// Enqueue a DMA command
  template <typename Cmd> void push_command(Cmd&& command) {
    ++nb_command;
    c.push(std::forward<Cmd>(command));
  }

 public:
  /// Start the DMA engine using an executor
  dma(::trisycl::detail::fiber_pool& fe) {
    data_mover = fe.submit([&] {
      dma_command sp;
      for (;;) {
        // Read a DMA command
        if (this->pop_command(sp) != boost::fibers::channel_op_status::success)
          // The channel is closed, stop working
          break;
        // Execute the command
        sp();
        commit_command();
      }
    });
  }

  /// Destructor handling the correct infrastructure shutdown
  ~dma() {
    // Close the command queue so the DMA does not accept any work
    c.close();
    /* Wait for the commands to drain to avoid calling
       std::terminate on destruction... */
    join();
  }

  /** Wait for the transfers to complete

      The API assumes that the program is written in such a way that
      at some point the queue will be empty so there is no continuous
      DMA command enqueuing for example. */
  SpecializedDMA& wait() {
    if (nb_command != 0) {
      // We need to wait because there are still operations executing
      std::unique_lock lk { waiting_mutex };
      waiting_room.wait(lk, [&] { return nb_command == 0; });
    }
    /* To be able to chain another DMA operation on it, return the DMA
       object itself */
    return static_cast<SpecializedDMA&>(*this);
  }
};

/** Receiving DMA

    \todo Factorize out the FIFO port
*/
template <typename AXIStreamSwitch>
class receiving_dma
    : public dma<AXIStreamSwitch, receiving_dma<AXIStreamSwitch>>
    , public communicator_port {
  using dma_base = dma<AXIStreamSwitch, receiving_dma<AXIStreamSwitch>>;

  /* boost::fibers::unbuffered_channel has no try_push() function, so
     use a buffered version for now

     \todo open a GitHub issue on Boost.Fiber
  */
  boost::fibers::buffered_channel<axi_packet> fifo { 8 };

  /// Keep track of the AXI stream switch owning this port for debugging
  AXIStreamSwitch& axi_ss;

 public:
  /// Start the receiving DMA engine using an executor
  receiving_dma(AXIStreamSwitch& axi_ss, ::trisycl::detail::fiber_pool& fe)
      : dma_base { fe }
      , axi_ss { axi_ss } {}

  /** Enqueue a DMA transfer to receive a span

      Use std::span for 1D contiguous transfers only for now
  */
  receiving_dma& receive(std::span<axi_packet::value_type> sp) {
    this->push_command([=, this] {
      /* Write each element received from input router port into the
         memory described by DMA operation */
      for (auto& e : sp) {
        e = read();
      }
    });
    return *this;
  }

  /// The network sends some data to the DMA receiver input
  void write(const axi_packet& v) override { fifo.push(v); }

  /** The network try to send some data to the DMA receiver input

      \return true if the packet is correctly enqueued or if the
      stream is closed, in order to to move on since we are in the
      tear-down phase of the infrastructure

      \todo think about it again...
  */
  bool try_write(const axi_packet& v) override {
    return this->fifo.try_push(v) != boost::fibers::channel_op_status::full;
  }

  /// Waiting read by a tile program on a core input port from the switch
  axi_packet::value_type read() override { return fifo.value_pop().data; }

  /** Non-blocking read to a core input port

      \return true if the value was correctly read
  */
  bool try_read(axi_packet::value_type& v) override {
    axi_packet p;

    if (fifo.try_pop(p) == boost::fibers::channel_op_status::success) {
      v = p.data;
      return true;
    }
    return false;
  }
};

/** Sending DMA

    \todo Factorize out the FIFO port
*/
template <typename AXIStreamSwitch>
class sending_dma : public dma<AXIStreamSwitch, sending_dma<AXIStreamSwitch>> {
  using dma_base = dma<AXIStreamSwitch, sending_dma<AXIStreamSwitch>>;

  std::shared_ptr<communicator_port> output_port;

 public:
  /// Start the DMA engine using an executor to push things on a port
  sending_dma(::trisycl::detail::fiber_pool& fe,
              std::shared_ptr<communicator_port> output)
      : dma_base { fe }
      , output_port { output } {}

  /// Enqueue a DMA transfer to send a span
  sending_dma& send(std::span<axi_packet::value_type> sp) {
    this->push_command([=, this] {
      // Send each element of this span to the output port
      for (const auto& e : sp) {
        output_port->write(e);
      }
    });
    return *this;
  }
};

/// @} End the aie Doxygen group

} // namespace trisycl::vendor::xilinx::acap::aie

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DMA_HPP
