#ifndef TRISYCL_SYCL_MARRAY_HPP
#define TRISYCL_SYCL_MARRAY_HPP

/** \file

    The SYCL marray class

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/vec.hpp"

namespace trisycl {
/** \addtogroup marray Math array types in SYCL

    @{
*/

/// A simplistic implementation for now
template <typename DataType, int numElements>
using marray = vec<DataType, numElements>;

/** A macro to define type alias, such as for type=uchar, size=4 and
    actual_type=unsigned char, muchar4 is equivalent to
    marray<unsigned char, 4>
*/
#define TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, size, actual_type)               \
  using m##type##size = marray<actual_type, size>;

/// Declare the vector types of a type for all the sizes
#define TRISYCL_DEFINE_MARRAY_TYPE(type, actual_type)                          \
  TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, 1, actual_type)                        \
  TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, 2, actual_type)                        \
  TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, 3, actual_type)                        \
  TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, 4, actual_type)                        \
  TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, 8, actual_type)                        \
  TRISYCL_DEFINE_MARRAY_TYPE_SIZE(type, 16, actual_type)

/// Declare all the possible marray type aliases
TRISYCL_DEFINE_MARRAY_TYPE(char, char)
TRISYCL_DEFINE_MARRAY_TYPE(uchar, unsigned char)
TRISYCL_DEFINE_MARRAY_TYPE(short, short int)
TRISYCL_DEFINE_MARRAY_TYPE(ushort, unsigned short int)
TRISYCL_DEFINE_MARRAY_TYPE(int, int)
TRISYCL_DEFINE_MARRAY_TYPE(uint, unsigned int)
TRISYCL_DEFINE_MARRAY_TYPE(long, long int)
TRISYCL_DEFINE_MARRAY_TYPE(ulong, unsigned long int)
TRISYCL_DEFINE_MARRAY_TYPE(float, float)
TRISYCL_DEFINE_MARRAY_TYPE(double, double)

#undef TRISYCL_DEFINE_MARRAY_TYPE
#undef TRISYCL_DEFINE_MARRAY_TYPE_SIZE

/// @} End the vector Doxygen group

} // namespace trisycl

#endif // TRISYCL_SYCL_MARRAY_HPP
