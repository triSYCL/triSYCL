#ifndef TRISYCL_SYCL_HALF_HPP
#define TRISYCL_SYCL_HALF_HPP

#include <cstring>
/** \file SYCL Half support - dummy
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

/* Dummy half implementation - this is enough to get the CTS to build. */
namespace cl {
namespace sycl {

namespace detail {
#include "CL/sycl/detail/half.hpp"
}

using half = detail::half_float::half;
#ifdef TRISYCL_OPENCL
using cl_half = detail::half_float::half;
#endif
}

}

#endif
