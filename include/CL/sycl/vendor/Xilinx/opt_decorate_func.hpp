#ifndef TRISYCL_SYCL_VENDOR_DECORATE_FUNC_HPP
#define TRISYCL_SYCL_VENDOR_DECORATE_FUNC_HPP

/** \file This file includes some decorating functions for Xilinx tools
    supported optimization.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {
namespace vendor {


/** Applying dataflow on functions

    dataflow function taking functions that user wants to apply dataflow on as
    arguments.
*/
template <typename Functor>
void dataflow(Functor f) noexcept {
  /* ssdm instruction is inserted before the argument functor to guide xocc to
     do dataflow. */
  _ssdm_op_SpecDataflowPipeline(-1, "");
  f();
}


/** Applying pipeline on loops

    pipeline function taking the loops which wraps in a lambda function that
    user wants to apply pipeline on as arguments.
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

#endif // TRISYCL_SYCL_VENDOR_DECORATE_FUNC_HPP
