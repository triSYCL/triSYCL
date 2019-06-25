#ifndef TRISYCL_SYCL_CONTEXT_DETAIL_CONTEXT_TAIL_HPP
#define TRISYCL_SYCL_CONTEXT_DETAIL_CONTEXT_TAIL_HPP

/** \file The ending part of of the OpenCL SYCL context

    This is here to break a dependence between context and device_selector

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl {

/* has to be inline as it references default_selector */
inline context::context(const vector_class<device> &deviceList,
			async_handler asyncHandler)
  : context(deviceList, asyncHandler, default_selector {}) {}

}
/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_CONTEXT_DETAIL_CONTEXT_TAIL_HPP
