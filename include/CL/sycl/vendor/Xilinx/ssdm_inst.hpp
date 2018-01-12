#ifndef TRISYCL_SYCL_VENDOR_XILINX_SSDM_INST_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_SSDM_INST_HPP


/** \file This file includes some of the SSDM Intrinsics supported in Xilinx
    tools.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/


#ifdef TRISYCL_DEVICE
extern "C" {
  /// SSDM Intrinsics: dataflow operation
  void _ssdm_op_SpecDataflowPipeline(...) __attribute__ ((nothrow, noinline, weak));
  /// SSDM Intrinsics: pipeline operation
  void _ssdm_op_SpecPipeline(...) __attribute__ ((nothrow, noinline, weak));
  /// SSDM Intrinsics: array partition operation
  void _ssdm_SpecArrayPartition(...) __attribute__ ((nothrow, noinline, weak));
}
#else
/// If not on device, just remove the intrinsics as defining them as empty macros
#define _ssdm_op_SpecDataflowPipeline(...)
#define _ssdm_op_SpecPipeline(...)
#define _ssdm_SpecArrayPartition(...)
#endif


/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VENDOR_XILINX_SSDM_INST_HPP
