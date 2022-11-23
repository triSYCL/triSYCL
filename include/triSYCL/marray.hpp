#ifndef TRISYCL_SYCL_MARRAY_HPP
#define TRISYCL_SYCL_MARRAY_HPP

/** \file

    The SYCL marray class

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/vec.hpp"

namespace trisycl {

/// A simplistic implementation for now
template <typename DataType, int numElements>
using marray = vec<DataType, numElements>;

}

#endif // TRISYCL_SYCL_MARRAY_HPP
