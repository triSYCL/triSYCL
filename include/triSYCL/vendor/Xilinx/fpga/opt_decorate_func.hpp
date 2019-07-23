#ifndef TRISYCL_SYCL_VENDOR_XILINX_FPGA_OPT_DECORATE_FUNC_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_FPGA_OPT_DECORATE_FUNC_HPP

/** \file This file includes some decorating functions for
    some optimizations supported by Xilinx tools.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

/** \addtogroup Xilinx Xilinx vendor extensions
    @{
*/

namespace trisycl::vendor::xilinx {

/** Apply dataflow execution on functions or loops

    With this mode, Xilinx tools analyze the dataflow dependencies
    between sequential functions or loops and create channels (based
    on ping-pong RAMs or FIFOs) that allow consumer functions or loops
    to start operation before the producer functions or loops have
    completed.

    This allows functions or loops to operate in parallel, which
    decreases latency and improves the throughput.

    \param[in] f is a function that functions or loops in f will be executed
    in a dataflow manner.
*/
auto dataflow = [] (auto functor) noexcept {
  /* SSDM instruction is inserted before the argument functor to guide xocc to
     do dataflow. */
  _ssdm_op_SpecDataflowPipeline(-1, "");
  functor();
};


/** Execute loops in a pipelined manner

    A loop with pipeline mode processes a new input every clock
    cycle. This allows the operations of different iterations of the
    loop to be executed in a concurrent manner to reduce latency.

    \param[in] f is a function with an innermost loop to be executed in a
    pipeline way.
*/
auto pipeline = [] (auto functor) noexcept {
  /* SSDM instruction is inserted before the argument functor to guide xocc to
     do pipeline. */
  _ssdm_op_SpecPipeline(1, 1, 0, 0, "");
  functor();
};

}

/// @} End the Xilinx Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_FPGA_OPT_DECORATE_FUNC_HPP
