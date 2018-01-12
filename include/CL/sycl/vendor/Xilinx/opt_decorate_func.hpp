#ifndef TRISYCL_SYCL_VENDOR_XILINX_OPT_DECORATE_FUNC_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_OPT_DECORATE_FUNC_HPP

/** \file This file includes some decorating functions for Xilinx tools
    supported optimization.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {
namespace xilinx {


/** Applying dataflow on functions or loops

    Xilinx tools analyzes the dataflow between sequential functions or loops
    and create channels (based on pingpong RAMs or FIFOs) that allow consumer
    functions or loops to start operation before the producer functions or
    loops have completed.

    This allows functions or loops to operate in parallel, which decreases
    latency and improves the
    throughput.

    \param[in] f is a function that functions or loops in f will be executed
    in a dataflow manner.
*/
template <typename Functor>
void dataflow(Functor f) noexcept {
  /* ssdm instruction is inserted before the argument functor to guide xocc to
     do dataflow. */
  _ssdm_op_SpecDataflowPipeline(-1, "");
  f();
}


/** Execute loops in a pipelined manner

    A loop with pipeline processes a new input every clock cycle. This allows
    the operations of the loop to be implemented in a concurrent manner.

    \param[in] f is a function with an innermost loop to be executed in a
    pipeline way.
*/
template <typename Functor>
void pipeline(Functor f) noexcept {
  /* ssdm instruction is inserted before the argument functor to guide xocc to
     do pipeline. */
  _ssdm_op_SpecPipeline(1, 1, 0, 0, "");
  f();
}


}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_OPT_DECORATE_FUNC_HPP
