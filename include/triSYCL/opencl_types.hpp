#ifndef TRISYCL_SYCL_OPENCL_TYPES_HPP
#define TRISYCL_SYCL_OPENCL_TYPES_HPP

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
#include "triSYCL/vec.hpp"
#endif


namespace trisycl {

// List of the cl_types that will be iterated upon to generate the vector types
#define TRISYCL_SCALAR_TYPES                                                    \
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

// Accessors to get the type name, the cl_type name and the actual type
#define TRISYCL_TYPE_NAME(T)         BOOST_PP_TUPLE_ELEM(3, 0, T)
#define TRISYCL_TYPE_CL_NAME(T)      BOOST_PP_TUPLE_ELEM(3, 1, T)
#define TRISYCL_TYPE_ACTUAL_NAME(T)  BOOST_PP_TUPLE_ELEM(3, 2, T)

// Return the name of the type concatenated with the size unless the size is 1
#define TRISYCL_SIZED_NAME(T, size)                                            \
  BOOST_PP_IF(                                                                 \
    BOOST_PP_EQUAL(size, 1), T,                                                \
    BOOST_PP_CAT(T, size))

#ifndef TRISYCL_OPENCL

/* If we are not using Boost Compute we just every cl_type with its \c vec
   equivalent
*/
#define TRISYCL_DEFINE_TYPES(scalar, i)                                        \
  using TRISYCL_SIZED_NAME(TRISYCL_TYPE_CL_NAME(scalar), i) =                  \
    BOOST_PP_IF(                                                               \
      BOOST_PP_EQUAL(i, 1),                                                    \
      TRISYCL_TYPE_ACTUAL_NAME(scalar),                                        \
      BOOST_PP_CAT(TRISYCL_TYPE_NAME(scalar), i));

#else

/* When passing the arguments to Boost Compute, we need to know if we have to
   unwrap our type and give Boost Compute the actual OpenCL type. To do this
   we define this meta-function that returns true if \c T is a wrapper type
   to an OpenCL type. For example \c is_wrapper<trisycl::int3>::value == true
*/
template<class T>
struct is_wrapper : std::false_type {};

#define TRISYCL_IS_WRAPPER_TRAIT(type)                                         \
template <> struct is_wrapper<type> : std::true_type {};

/* We define 3 different wrapper classes around OpenCL types.
   These classes allow us to use t.x() and t.y() for vector types of size 2.
   And in addition we can also use t.z() and t.w() for vector types of size
   3 and 4 similarly to OpenCL vector types
*/
#define TRISYCL_WRAPPER_CLASS_2(cl_type, boost_name, scalar_type)              \
  class cl_type {                                                              \
    ::cl_type self;                                                            \
                                                                               \
  public:                                                                      \
    cl_type () = default;                                                      \
    cl_type (::cl_type self_) : self { self_ } {}                              \
    cl_type (scalar_type x, scalar_type y) : self { x, y } {}                  \
    auto& x() { return self.s[0]; }                                            \
    auto& y() { return self.s[1]; }                                            \
    auto& unwrap() const { return self; }};                                    \
  TRISYCL_IS_WRAPPER_TRAIT(cl_type)

#define TRISYCL_WRAPPER_CLASS_3(cl_type, boost_name, scalar_type)              \
  class cl_type {                                                              \
    ::cl_type self;                                                            \
                                                                               \
  public:                                                                      \
    cl_type () = default;                                                      \
    cl_type (::cl_type self_) : self { self_ } {}                              \
    cl_type (scalar_type x, scalar_type y, scalar_type z) :                    \
      self { x, y, z } {}                                                      \
    auto& x() { return self.s[0]; }                                            \
    auto& y() { return self.s[1]; }                                            \
    auto& z() { return self.s[2]; }                                            \
    auto& unwrap() const { return self; }};                                    \
  TRISYCL_IS_WRAPPER_TRAIT(cl_type)

#define TRISYCL_WRAPPER_CLASS_4(cl_type, boost_name, scalar_type)              \
  class cl_type {                                                              \
    ::cl_type self;                                                            \
                                                                               \
  public:                                                                      \
    cl_type () = default;                                                      \
    cl_type (::cl_type self_) : self { self_ } {}                              \
    cl_type (scalar_type x, scalar_type y, scalar_type z, scalar_type w) :     \
      self { x, y, z, w } {}                                                   \
    auto& x() { return self.s[0]; }                                            \
    auto& y() { return self.s[1]; }                                            \
    auto& z() { return self.s[2]; }                                            \
    auto& w() { return self.s[3]; }                                            \
    auto& unwrap() const { return self; }};                                    \
  TRISYCL_IS_WRAPPER_TRAIT(cl_type)


// Return the Boost Compute type for OpenCL vector types
#define TRISYCL_BOOST_COMPUTE_NAME(scalar, size)                               \
  TRISYCL_SIZED_NAME(boost::compute::TRISYCL_TYPE_NAME(scalar), size)

/* For vector types of size above 4 and 1 we typedef the vector type with
   its Boost Compute equivalent
*/
#define TRISYCL_TYPEDEF_TYPE(cl_type, boost_name, scalar_type)                  \
  using cl_type = boost_name##_;

// Helper macro to properly define each OpenCL type
#define TRISYCL_H_DEFINE_TYPE(cl_type, boost_name, scalar_type, i)             \
  BOOST_PP_IF(BOOST_PP_EQUAL(i, 1), TRISYCL_TYPEDEF_TYPE,                      \
              BOOST_PP_IF(BOOST_PP_EQUAL(i, 2), TRISYCL_WRAPPER_CLASS_2,       \
                          BOOST_PP_IF(BOOST_PP_EQUAL(i, 3),                    \
                                      TRISYCL_WRAPPER_CLASS_3,                 \
                                      BOOST_PP_IF(BOOST_PP_EQUAL(i, 4),        \
                                                  TRISYCL_WRAPPER_CLASS_4,     \
                                                  TRISYCL_TYPEDEF_TYPE))))     \
  (cl_type, boost_name, scalar_type)

// Helper macro to properly define each OpenCL type
#define TRISYCL_DEFINE_TYPES(scalar, i)                                        \
  TRISYCL_H_DEFINE_TYPE(TRISYCL_SIZED_NAME(TRISYCL_TYPE_CL_NAME(scalar), i),   \
                        TRISYCL_BOOST_COMPUTE_NAME(scalar, i),                 \
                        TRISYCL_TYPE_ACTUAL_NAME(scalar), i)

#endif

/* We declare vector types of sizes 1,2,3,4,8 and 16 as per
   the SYCL specification
*/
#define TRISYCL_DECLARE_CL_TYPES(r, data, scalar)                              \
  TRISYCL_DEFINE_TYPES(scalar, 1)                                              \
  TRISYCL_DEFINE_TYPES(scalar, 2)                                              \
  TRISYCL_DEFINE_TYPES(scalar, 3)                                              \
  TRISYCL_DEFINE_TYPES(scalar, 4)                                              \
  TRISYCL_DEFINE_TYPES(scalar, 8)                                              \
  TRISYCL_DEFINE_TYPES(scalar, 16)

// Generate the vector types for all listed scalar types
BOOST_PP_LIST_FOR_EACH(TRISYCL_DECLARE_CL_TYPES, _, TRISYCL_SCALAR_TYPES)

} // cl


// Undef macros to avoid name collision
#undef TRISYCL_SCALAR_TYPES
#undef TRISYCL_TYPE_NAME
#undef TRISYCL_TYPE_CL_NAME
#undef TRISYCL_TYPE_ACTUAL_NAME
#undef TRISYCL_SIZED_NAME
#undef TRISYCL_IS_WRAPPER_TRAIT
#undef TRISYCL_WRAPPER_CLASS_2
#undef TRISYCL_WRAPPER_CLASS_3
#undef TRISYCL_WRAPPER_CLASS_4
#undef TRISYCL_BOOST_COMPUTE_NAME
#undef TRISYCL_TYPEDEF_TYPE
#undef TRISYCL_DEFINE_TYPES
#undef TRISYCL_DECLARE_CL_TYPES

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_OPENCL_TYPES_HPP
