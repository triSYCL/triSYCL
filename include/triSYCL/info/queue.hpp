#ifndef TRISYCL_SYCL_INFO_QUEUE_HPP
#define TRISYCL_SYCL_INFO_QUEUE_HPP

/** \file The OpenCL SYCL queue information parameters

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/id.hpp"
#include "triSYCL/opencl_types.hpp"
#include "triSYCL/info/param_traits.hpp"

namespace trisycl::info {

using queue_profiling = bool;

/** Queue information descriptors
 */
enum class queue : int {
  context,
  device,
  reference_count,
};

TRISYCL_INFO_PARAM_TRAITS(queue::context, trisycl::context)
TRISYCL_INFO_PARAM_TRAITS(queue::device, trisycl::device)
TRISYCL_INFO_PARAM_TRAITS(queue::reference_count, trisycl::cl_uint)

}
#endif
