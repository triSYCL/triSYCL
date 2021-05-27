#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP

/** \file

    SYCL-friendly queue for AIE cooperative program including all the tiles

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <future>
#include <utility>

#include "program.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine minimal queue

    \todo Make a real queue implementation instead of this mock-up

    \param AIEDevice is the AIE device targeted by the queue
*/
template <typename AIEDevice>
struct queue {
  using device = AIEDevice;
  using geo = typename device::geo;
  using layout = typename geo::layout;

  /// \todo Why using a reference since now a device is a shared_ptr?
  device& aie_d;

  queue(device &d) : aie_d { d } {}


  /** Provide a wait() function that actually can throw an exception
      instead of ignoring it by the std::future::wait() */
  struct queue_waiter : std::future<void> {
    void wait() {
      // Just call the function unwrapping the promise, it will
      // rethrow if there is any exception pending
      get();
    }
  };


  /** Run synchronously a program execution on this queue

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename Device,
                      int X,
                      int Y> typename Tile = acap::aie::tile,
            template <typename Device,
                      int X,
                      int Y> typename Memory = acap::aie::memory>
  void run() const {
    program<device, Tile, Memory> { aie_d }.run();
  }


  /** Run synchronously an heterogeneous invocable on this queue

      \param f is an invocable taking an heterogeneous tile handler
  */
  template <typename Invocable>
  void run(Invocable&& f) const {
    program<device> { aie_d }.run(std::forward<Invocable>(f));
  }


  /// Wait for all the device tiles on this queue to finish
  void wait() {
    aie_d.for_each_tile([](auto& t) { t.wait(); });
  }

  /** Run synchronously a uniform invocable on this queue

      \param f is an invocable taking a uniform tile handler
  */
  template <typename Invocable>
  void uniform_run(Invocable&& f) const {
    aie_d.for_each_tile(std::forward<Invocable>(f));
  }


///\todo SUBMIT

  /** Submit a program execution on this queue

      \param Tile is the description of the program tiles to
      instantiate. By default each tile will run an empty program.

      \param Memory is the description of the machine memory modules. By
      default the machine has empty memory modules.
  */
  template <template <typename Device,
                      int X,
                      int Y> typename Tile = acap::aie::tile,
            template <typename Device,
                      int X,
                      int Y> typename Memory = acap::aie::memory>
  auto submit() const {
    return queue_waiter {
      std::async(std::launch::async,
                 /** \fixme implement a real queue with lifetime
                     support because here it assume the device will
                     last... */
                 [*this] { this->run<Tile, Memory>(); }) };
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

#endif // TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP
