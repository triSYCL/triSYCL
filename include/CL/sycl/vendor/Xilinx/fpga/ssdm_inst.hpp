#ifndef TRISYCL_SYCL_VENDOR_XILINX_FPGA_SSDM_INST_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_FPGA_SSDM_INST_HPP

/** \file This file defines some of the SSDM intrinsics used in Xilinx
    tools.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

/** \addtogroup Xilinx Xilinx vendor extensions
    @{
*/

#ifdef TRISYCL_DEVICE
extern "C" {
  /// SSDM Intrinsics: dataflow operation
  void _ssdm_op_SpecDataflowPipeline(...)
    __attribute__ ((nothrow, noinline, weak));
  /// SSDM Intrinsics: pipeline operation
  void _ssdm_op_SpecPipeline(...) __attribute__ ((nothrow, noinline, weak));
  /// SSDM Intrinsics: array partition operation
  void _ssdm_SpecArrayPartition(...) __attribute__ ((nothrow, noinline, weak));
}
#else
/* If not on device, just ignore the intrinsics as defining them as
   empty variadic macros replaced by an empty do-while to avoid some
   warning when compiling (and used in an if branch */
#define _ssdm_op_SpecDataflowPipeline(...) do { } while (0)
#define _ssdm_op_SpecPipeline(...) do { } while (0)
#define _ssdm_SpecArrayPartition(...) do { } while (0)
#endif

/// @} End the Xilinx Doxygen group

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_FPGA_SSDM_INST_HPP
