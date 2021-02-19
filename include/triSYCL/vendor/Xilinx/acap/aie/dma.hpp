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

#include <boost/fiber/buffered_channel.hpp>
#include <boost/format.hpp>
#include "magic_enum.hpp"

#include <range/v3/all.hpp>

#include "connection.hpp"
#include "triSYCL/detail/enum.hpp"
#include "triSYCL/detail/fiber_pool.hpp"
#include "triSYCL/vendor/Xilinx/latex.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/// Generic DMA
template <typename AXIStreamSwitch, typename SpecializedDMA>
class dma : detail::debug<SpecializedDMA> {

public:

  /// Use std::span for 1D contiguous transfers only for now
  using dma_command = std::span<axi_packet::value_type>;

  /// DMA command queue
  boost::fibers::buffered_channel<dma_command> c { 8 };

  /// To shepherd the fiber playing the data mover behind the DMA
  detail::fiber_pool::future<void> data_mover;

  /// Count the in-flight commands
  std::atomic<int> nb_command { 0 };

  /// To wait for the DMA command queue to drain
  boost::fibers::condition_variable waiting_room;

  /// To protect the waiting mechanism
  boost::fibers::mutex waiting_mutex;

  /// Wait for the fiber to complete
  void join() {
std::cerr << "dma join()" << std::endl;
    // Get the value of the future, to get an exception if any
    data_mover.get();
std::cerr << "dma joined()" << std::endl;
  }


public:

  /// Destructor handling the correct infrastructure shutdown
  ~dma() {
std::cerr << "~dma()" << std::endl;
      // Close the command queue so the DMA does not accept any work
      //c.close();
std::cerr << "~dma() c.close(), joining..." << std::endl;
      /* Wait for the commands to drain to avoid calling
         std::terminate on destruction... */
      join();
std::cerr << "~dma() done!" << std::endl;
    }


  /// Launch the data mover on the execution pool
  template <typename Invokable>
  void launch(detail::fiber_pool &fiber_executor, Invokable&& f) {
std::cerr << "dma launching..." << std::endl;
    data_mover = fiber_executor.submit(std::forward<Invokable>(f));
std::cerr << "dma launched!" << std::endl;
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
    return *static_cast<SpecializedDMA *>(this);
  }

protected:

  /// Enqueue a DMA command
  void push(const dma_command& cmd) {
    ++nb_command;
try {
    c.push(cmd);
} catch(...) { std::cerr << "dma push throwed!" << std::endl;}
  }


  /// Dequeue a DMA command
  auto pop() {
    typename decltype(c)::value_type command;
try {
    command = c.value_pop();
    //auto command = c.value_pop();
} catch(...) { std::cerr << "dma pop throwed!" << std::endl;}
    return command;
  }


  /// Notify a command has been processed
  void commit_command() {
    if (--nb_command == 0)
      // Warn anyone waiting for the DMA queue to drain it might be empty now
      waiting_room.notify_all();
  }
};


/** Receiving DMA

    \todo Factorize out the FIFO port
*/
template <typename AXIStreamSwitch>
class receiving_dma : public dma<AXIStreamSwitch,
                                 receiving_dma<AXIStreamSwitch>>,
                      public AXIStreamSwitch::router_port {
  using dma_base = dma<AXIStreamSwitch, receiving_dma<AXIStreamSwitch>>;

  /* boost::fibers::unbuffered_channel has no try_push() function, so
     use a buffered version for now

     \todo open a GitHub issue on Boost.Fiber
  */
  boost::fibers::buffered_channel<axi_packet> fifo { 8 };

public:

  /// Start the DMA engine using an executor
  receiving_dma(AXIStreamSwitch& axi_ss, detail::fiber_pool &fe)
    : AXIStreamSwitch::router_port { axi_ss } {
    this->launch(fe, [&] {
std::cerr << "receiving_dma fiber starting" << std::endl;
                       typename dma_base::dma_command sp;
                       for(;;) {
std::cerr << "receiving_dma Read a DMA command" << std::endl;
                         /* Read a DMA command which is a span
                            expressing memory destination */
                         if (this->c.pop(sp)
                             != boost::fibers::channel_op_status::success)
                           // The channel is closed, stop working
                           break;
                         /* Write each element received from input
                            router port into the memory described by
                            DMA operation */
                         for (auto& e : sp) {
                           std::cerr << "receiving_dma read before" << std::endl;
                           e = read();
                           std::cerr << "receiving_dma read after " << e << std::endl;
                         }
                         this->commit_command();
                       }
std::cerr << "receiving_dma fiber ending" << std::endl;
                     });
  }


  /// The network sends some data to the DMA receiver input
  void write(const axi_packet &v) override {
try {
    fifo.push(v);
} catch(...) { std::cerr << "receiving_dma::write throwed!" << std::endl;}
  }


  /** The network try to send some data to the DMA receiver input

      \return true if the packet is correctly enqueued or if the
      stream is closed, in order to to move on since we are in the
      tear-down phase of the infrastructure

      \todo think about it again...
  */
  bool try_write(const axi_packet &v) override {
    return this->fifo.try_push(v) != boost::fibers::channel_op_status::full;
  }


  /// Waiting read by a tile program on a core input port from the switch
  axi_packet::value_type read() override {
try {
    return fifo.value_pop().data;
} catch(const boost::fibers::fiber_error& e) {
  std::cerr << "receiving_dma::read throwed! " << e.what()
            << (void*) this << std::endl;} return {};
  }


  /** Non-blocking read to a core input port

      \return true if the value was correctly read
  */
  bool try_read(axi_packet::value_type &v) override {
    axi_packet p;

    if (fifo.try_pop(p) == boost::fibers::channel_op_status::success) {
      v = p.data;
      return true;
    }
    return false;
  }


  /// Enqueue a DMA transfer to receive a span
  auto& receive(typename dma_base::dma_command cmd) {
try {
    this->c.push(std::move(cmd));
} catch(...) { std::cerr << "receiving_dma::receive throwed!" << std::endl;}
    // To be able to chain another DMA operation on it
    return *this;
  }
};


/** Sending DMA

    \todo Factorize out the FIFO port
*/
template <typename AXIStreamSwitch>
class sending_dma : public dma<AXIStreamSwitch, sending_dma<AXIStreamSwitch>> {
  using dma_base = dma<AXIStreamSwitch, sending_dma<AXIStreamSwitch>>;

public:

  /// Start the DMA engine using an executor to push things on a port
  sending_dma(detail::fiber_pool &fe,
              std::shared_ptr<typename AXIStreamSwitch::router_port> output) {
std::cerr << "sending_dma fiber starting" << std::endl;
    this->launch(fe, [o = std::move(output), this] {
                       typename dma_base::dma_command sp;
                       for(;;) {
std::cerr << "sending_dma Read a DMA command" << std::endl;
                         /* Read a DMA command which is a span
                            describing what to read from memory */
                         if (this->c.pop(sp)
                             != boost::fibers::channel_op_status::success)
                           // The channel is closed, stop working
                           break;
                         /* Send each element of this span to the output
                            port */
                         for (const auto& e : sp) {
                           std::cerr << "sending_dma write before " << e << std::endl;
                           o->write(e);
                           std::cerr << "sending_dma write after " << e << std::endl;
                         }
                         this->commit_command();
                       }
std::cerr << "sending_dma fiber ending" << std::endl;
                     });
  }


  /// Enqueue a DMA transfer to send a span
  auto& send(const typename dma_base::dma_command& cmd) {
try {
  std::cerr << "sending_dma c.push(cmd)" << std::endl;
    this->c.push(cmd);
} catch(...) { std::cerr << "sending_dma::send throwed!" << std::endl;}
    // To be able to chain another DMA operation on it
    return *this;
  }

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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_DMA_HPP
