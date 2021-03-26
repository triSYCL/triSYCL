#ifndef TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_ACAP_AIE_QUEUE_HPP

/** \file

    SYCL-friendly queue for AIE

    Ronan dot Keryell at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <future>

#include "program.hpp"

namespace trisycl::vendor::xilinx::acap::aie {

/// \ingroup aie
/// @{

/** The AI Engine minimal queue

    \param AIEDevice is the AIE device targeted by the queue
*/
template <typename AIEDevice>
struct queue {
  using geo = typename AIEDevice::geo;
  using layout = typename geo::layout;

  AIEDevice &aie_d;

  queue(AIEDevice &d) : aie_d { d } {}


  /** Provide a wait() function that actually can throw an exception
      instead of ignoring it by the std::future::wait() */
  struct queue_waiter : std::future<void> {
    void wait() {
      // Just call the function unwrapping the promise, it will
      // rethrow if there is any exception pending
      get();
    }
  };


  /** Run synchronously a program on this queue

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
    program<AIEDevice, Tile, Memory> { aie_d }.run();
  }


  /** Run synchronously an heterogeneous invokable on this queue

      \param f is an invokable taking an heterogeneous tile handler
  */
  template <typename Invokable>
  void run(const Invokable& f) const {
    program { f }.run();
  }


  /** Run synchronously a uniform invokable on this queue

      \param f is an invokable taking a uniform tile handler
  */
  template <typename Invokable>
  void uniform_run(const Invokable& f) const {
    program { f }.uniform_run(f);
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
