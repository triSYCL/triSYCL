#ifndef TRISYCL_SYCL_OPENCL_TYPE_HPP
#define TRISYCL_SYCL_OPENCL_TYPE_HPP

/** \file
    triSYCL wrapper for OpenCL types

    Joan DOT Thibault AT ens-rennes DOT fr
    a.doumoulakis AT gmail DOT com

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/logical/not.hpp>
#include <boost/preprocessor/logical/or.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#ifdef TRISYCL_OPENCL
#include <boost/compute/types/fundamental.hpp>
#else
#include "CL/sycl/vec.hpp"
#endif


namespace cl {
namespace sycl {

#define SCALAR_TYPES                                                           \
  BOOST_PP_TUPLE_TO_LIST(                                                      \
    10,                                                                        \
    (                                                                          \
      ( char   ,cl_char,   char),                                              \
      ( uchar  ,cl_uchar,  unsigned char),                                     \
      ( short  ,cl_short,  short int),                                         \
      ( ushort ,cl_ushort, unsigned short int),                                \
      ( int    ,cl_int,    int),                                               \
      ( uint   ,cl_uint,   unsigned int),                                      \
      ( long   ,cl_long,   long int),                                          \
      ( ulong  ,cl_ulong,  unsigned long int),                                 \
      ( float  ,cl_float,  float),                                             \
      ( double ,cl_double, double)                                             \
      )                                                                        \
    )

#define TYPE_NAME(T)         BOOST_PP_TUPLE_ELEM(3, 0, T)
#define TYPE_CL_NAME(T)      BOOST_PP_TUPLE_ELEM(3, 1, T)
#define TYPE_ACTUAL_NAME(T)  BOOST_PP_TUPLE_ELEM(3, 2, T)

#define SIZED_NAME(T, size)                      \
  BOOST_PP_IF(                                   \
    BOOST_PP_EQUAL(size, 1), T,                  \
    BOOST_PP_CAT(T, size))

#ifndef TRISYCL_OPENCL

#define DEFINE_TYPES(scalar, i)                  \
  using SIZED_NAME(TYPE_CL_NAME(scalar), i) =    \
    BOOST_PP_CAT(TYPE_NAME(scalar), i);

#else

#define WRAPPER_CLASS_1(cl_type, boost_name, scalar_type)                      \
  class cl_type {                                                              \
    boost_name##_ self;                                                        \
                                                                               \
  public:                                                                      \
    cl_type () = default;                                                      \
    cl_type (boost_name##_ self_) { self = self_; }                            \
    cl_type (scalar_type x) { self = x; }                                      \
    auto& x() { return self; }};

#define WRAPPER_CLASS_2(cl_type, boost_name, scalar_type)                      \
  class cl_type {                                                              \
    boost_name##_ self;                                                        \
                                                                               \
  public:                                                                      \
    cl_type () = default;                                                      \
    cl_type (boost_name##_ self_) : self { self_ } {}                          \
    cl_type (scalar_type x, scalar_type y) : self { x, y } {}                  \
    auto& x() { return self[0]; }                                              \
    auto& y() { return self[1]; }};

#define WRAPPER_CLASS_3(cl_type, boost_name, scalar_type)                      \
  class cl_type {                                                              \
    boost_name##_ self;                                                        \
                                                                               \
  public:                                                                      \
    cl_type () = default;                                                      \
    cl_type (boost_name##_ self_) : self { self_ } {}                          \
    cl_type (scalar_type x, scalar_type y, scalar_type z) :                    \
      self { x, y, z, 0 } {}                                                   \
    auto& x() { return self[0]; }                                              \
    auto& y() { return self[1]; }                                              \
    auto& z() { return self[2]; }};


#define CAT_SIZED_NAME_BOOST_COMPUTE(scalar, size)                      \
  SIZED_NAME(boost::compute::TYPE_NAME(scalar), size)

#define CAT_SIZED_NAME_BOOST_COMPUTE_4(scalar, size)                    \
  SIZED_NAME(boost::compute::TYPE_NAME(scalar), 4)

#define TYPEDEF_TYPE(cl_type, boost_name, scalar_type)                  \
  using cl_type = boost_name##_;

#define P_DEFINE_TYPE(cl_type, boost_name, i, scalar_type)               \
  BOOST_PP_IF(BOOST_PP_EQUAL(i, 1), WRAPPER_CLASS_1,                     \
              BOOST_PP_IF(BOOST_PP_EQUAL(i, 2), WRAPPER_CLASS_2,         \
                          BOOST_PP_IF(BOOST_PP_EQUAL(i, 3),              \
                                      WRAPPER_CLASS_3, TYPEDEF_TYPE)))   \
  (cl_type, boost_name, scalar_type)

#define DEFINE_TYPES(scalar, i)                                       \
  DEFINE_TYPE(SIZED_NAME(TYPE_CL_NAME(scalar), i),                       \
              BOOST_PP_IF(BOOST_PP_EQUAL(i, 3),                          \
                          CAT_SIZED_NAME_BOOST_COMPUTE_4,                \
                          CAT_SIZED_NAME_BOOST_COMPUTE)(scalar, i),      \
              i, TYPE_ACTUAL_NAME(scalar))

#endif

#define DECLARE_CL_TYPES(r, data, scalar)            \
  DEFINE_TYPES(scalar, 1)                            \
  DEFINE_TYPES(scalar, 2)                            \
  DEFINE_TYPES(scalar, 3)                            \
  DEFINE_TYPES(scalar, 4)                            \
  DEFINE_TYPES(scalar, 8)                            \
  DEFINE_TYPES(scalar, 16)

BOOST_PP_LIST_FOR_EACH(DECLARE_CL_TYPES, _, SCALAR_TYPES)

#undef SCALAR_TYPES
#undef TYPE_NAME
#undef TYPE_CL_NAME
#undef SIZED_NAME
#undef TYPEDEF_TYPE
#undef DECLARE_CL_TYPES

} // sycl
} // cl

#endif // TRISYCL_SYCL_OPENCL_TYPE_HPP
