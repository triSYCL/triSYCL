#ifndef TRISYCL_SYCL_INFO_CONTEXT_HPP
#define TRISYCL_SYCL_INFO_CONTEXT_HPP

/** \file The OpenCL SYCL context information parameters

    Anastasi at Xilinx dot com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/info/param_traits.hpp"

namespace trisycl {

/** \addtogroup execution Platforms, contexts, devices and queues
    @{
*/
namespace info {

/** Context information descriptors

    \todo Should be unsigned int to be consistent with others?
*/
enum class context : int {
  reference_count,
  platform,
  devices
};


/** Query the return type for get_info() on context stuff

    \todo To be implemented, return always void.
*/
TRISYCL_INFO_PARAM_TRAITS_ANY_T(info::context, void)
TRISYCL_INFO_PARAM_TRAITS(info::context::reference_count, trisycl::cl_uint)
TRISYCL_INFO_PARAM_TRAITS(info::context::platform, trisycl::platform)
TRISYCL_INFO_PARAM_TRAITS(info::context::devices, vector_class<trisycl::device>)
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/


#endif //TRISYCL_SYCL_INFO_CONTEXT_HPP
