#ifndef TRISYCL_SYCL_OPENCL_TYPES_HPP
#define TRISYCL_SYCL_OPENCL_TYPES_HPP

#include "CL/sycl/vec.hpp"

namespace cl {
namespace sycl {

#define SCALAR_TYPES                             \
  BOOST_PP_TUPLE_TO_LIST(                        \
    10,                                          \
    (                                            \
      ( char   ,cl_char),                        \
      ( uchar  ,cl_uchar),                       \
      ( short  ,cl_short),                       \
      ( ushort ,cl_ushort),                      \
      ( int    ,cl_int),                         \
      ( uint   ,cl_uint),                        \
      ( long   ,cl_long),                        \
      ( ulong  ,cl_ulong),                       \
      ( float  ,cl_float),                       \
      ( double ,cl_double)                       \
      )                                          \
    )

#define TYPE_NAME(T)         BOOST_PP_TUPLE_ELEM(2, 0, T)
#define TYPE_CL_NAME(T)      BOOST_PP_TUPLE_ELEM(2, 1, T)

#define SIZED_NAME(T, size)                      \
  BOOST_PP_IF(                                   \
    BOOST_PP_EQUAL(size, 1), T,                  \
    BOOST_PP_CAT(T, size))

#define TYPEDEF_TYPE(scalar, i)                  \
  using SIZED_NAME(TYPE_CL_NAME(scalar), i) =    \
        BOOST_PP_CAT(TYPE_NAME(scalar), i);

#define DECLARE_CL_TYPES(r, data, scalar)        \
  TYPEDEF_TYPE(scalar, 1)                        \
  TYPEDEF_TYPE(scalar, 2)                        \
  TYPEDEF_TYPE(scalar, 3)                        \
  TYPEDEF_TYPE(scalar, 4)                        \
  TYPEDEF_TYPE(scalar, 8)                        \
  TYPEDEF_TYPE(scalar, 16)                       \

BOOST_PP_LIST_FOR_EACH(DECLARE_CL_TYPES, _, SCALAR_TYPES)

} // sycl
} // cl

#undef SCALAR_TYPES
#undef TYPE_NAME
#undef TYPE_CL_NAME
#undef SIZED_NAME
#undef TYPEDEF_TYPE
#undef DECLARE_CL_TYPES

#endif // TRISYCL_SYCL_OPENCL_TYPES_HPP
