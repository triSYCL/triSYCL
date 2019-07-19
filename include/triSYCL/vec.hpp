#ifndef TRISYCL_SYCL_VEC_HPP
#define TRISYCL_SYCL_VEC_HPP

/** \file

    Implement the small OpenCL vector class

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace trisycl {

/** Rounding mode for vector conversions.
 */
enum class rounding_mode {
  automatic, /// Default rounding mode, rtz for integers, rte for floating-point
  rte,       /// Round to nearest even
  rtz,       /// Round towards zero
  rtp,       /// Round towards positive infinity
  rtn        /// Round towards negative infinity
};

/** Series of values to specify named swizzle indexes
 *  Used when calling the swizzle member function template.
 */
struct elem {
  static constexpr int x = 0;
  static constexpr int y = 1;
  static constexpr int z = 2;
  static constexpr int w = 3;
  static constexpr int r = 0;
  static constexpr int g = 1;
  static constexpr int b = 2;
  static constexpr int a = 3;
  static constexpr int s0 = 0;
  static constexpr int s1 = 1;
  static constexpr int s2 = 2;
  static constexpr int s3 = 3;
  static constexpr int s4 = 4;
  static constexpr int s5 = 5;
  static constexpr int s6 = 6;
  static constexpr int s7 = 7;
  static constexpr int s8 = 8;
  static constexpr int s9 = 9;
  static constexpr int sA = 10;
  static constexpr int sB = 11;
  static constexpr int sC = 12;
  static constexpr int sD = 13;
  static constexpr int sE = 14;
  static constexpr int sF = 15;
};

/** forward decl to use in the detail class
 */
template<typename, int>
class vec;

template <typename DataType, int numElements>
using __swizzled_vec__ = vec<DataType, numElements>;

}


/** \addtogroup vector Vector types in SYCL

    @{
*/


/** Small OpenCL vector class
*/

#include "triSYCL/detail/alignment_helper.hpp"
#include "triSYCL/vec/detail/vec.hpp"

namespace trisycl {
  /** Accessors to access hex indexed elements of a vector
   */
#define TRISYCL_DECLARE_S(x)                          \
  const DataType& s##x() const {                      \
    return (*this)[0x##x];                            \
  }                                                   \
                                                      \
  DataType& s##x() {                                  \
    return (*this)[0x##x];                            \
  }

#define TRISYCL_GEN_SWIZ2(str,idx0,idx1)              \
  const __swizzled_vec__<DataType, 2> str() const {   \
     return base_vec::swizzle(idx0, idx1);            \
  }                                                   \
  __swizzled_vec__<DataType, 2> str() {               \
    return base_vec::swizzle(idx0, idx1);             \
  }
#define TRISYCL_GEN_SWIZ3(str,idx0,idx1,idx2)         \
  const __swizzled_vec__<DataType, 3> str() const {   \
    return base_vec::swizzle(idx0, idx1, idx2);       \
  }                                                   \
  __swizzled_vec__<DataType, 3> str() {               \
    return base_vec::swizzle(idx0, idx1, idx2);       \
  }
#define TRISYCL_GEN_SWIZ4(str,idx0,idx1,idx2,idx3)    \
  const __swizzled_vec__<DataType, 4> str() const {   \
    return base_vec::swizzle(idx0, idx1, idx2, idx3); \
  }                                                   \
  __swizzled_vec__<DataType, 4> str() {               \
    return base_vec::swizzle(idx0, idx1, idx2, idx3); \
  }

template<typename DataType>
class alignas(detail::alignment_v<::trisycl::vec<DataType, 1>>)
  vec<DataType, 1> : public detail::vec<DataType, 1> {
  using base_vec = detail::vec<DataType, 1>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** Accessors to the first element of a vector
   */
  const DataType& x() const {
    return (*this)[0];
  }

  DataType& x() {
    return (*this)[0];
  }

  TRISYCL_DECLARE_S(0);

  operator DataType() const {
    return (*this)[0];
  }

  operator DataType() {
    return (*this)[0];
  }
};

template<typename DataType>
class alignas(detail::alignment_v<::trisycl::vec<DataType, 2>>)
  vec<DataType, 2> : public detail::vec<DataType, 2> {
  using base_vec = detail::vec<DataType, 2>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** Accessors to the first element of a vector
   */
  const DataType& x() const {
    return (*this)[0];
  }

  DataType& x() {
    return (*this)[0];
  }

  /** Accessors to the second element of a vector
   */
  const DataType& y() const {
    return (*this)[1];
  }

  DataType& y() {
    return (*this)[1];
  }

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);

  const __swizzled_vec__<DataType, 1> lo() const {
    return base_vec::swizzle(elem::s0);
  }

  __swizzled_vec__<DataType, 1> lo() {
    return base_vec::swizzle(elem::s0);
  }

  const __swizzled_vec__<DataType, 1> hi() const {
    return base_vec::swizzle(elem::s1);
  }

  __swizzled_vec__<DataType, 1> hi() {
    return base_vec::swizzle(elem::s1);
  }

  const __swizzled_vec__<DataType, 1> odd() const {
    return base_vec::swizzle(elem::s1);
  }

  __swizzled_vec__<DataType, 1> odd() {
    return base_vec::swizzle(elem::s1);
  }

  const __swizzled_vec__<DataType, 1> even() const {
    return base_vec::swizzle(elem::s0);
  }

  __swizzled_vec__<DataType, 1> even() {
    return base_vec::swizzle(elem::s0);
  }
#include "triSYCL/vec/detail/swiz2.hpp"
};

template<typename DataType>
class alignas(detail::alignment_v<::trisycl::vec<DataType, 3>>)
  vec<DataType, 3> : public detail::vec<DataType, 3> {
  using base_vec = detail::vec<DataType, 3>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** Accessors to the first element of a vector
   */
  const DataType& x() const {
    return (*this)[0];
  }

  DataType& x() {
    return (*this)[0];
  }

  /** Accessors to the second element of a vector
   */
  const DataType& y() const {
    return (*this)[1];
  }

  DataType& y() {
    return (*this)[1];
  }

  /** Accessors to the third element of a vector
   */
  const DataType& z() const {
     return (*this)[2];
  }

  DataType& z() {
    return (*this)[2];
  }

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
  TRISYCL_DECLARE_S(2);

  const __swizzled_vec__<DataType, 2> lo() const {
    return base_vec::swizzle(elem::s0, elem::s1);
  }

  __swizzled_vec__<DataType, 2> lo()  {
    return base_vec::swizzle(elem::s0, elem::s1);
  }

  const __swizzled_vec__<DataType, 2> hi() const {
    return base_vec::swizzle(elem::s2, elem::s2);
  }

  __swizzled_vec__<DataType, 2> hi() {
    return base_vec::swizzle(elem::s2, elem::s2);
  }

  const __swizzled_vec__<DataType, 2> odd() const {
    return base_vec::swizzle(elem::s1, elem::s1);
  }

  __swizzled_vec__<DataType, 2> odd() {
    return base_vec::swizzle(elem::s1, elem::s1);
  }

  const __swizzled_vec__<DataType, 2> even() const {
    return base_vec::swizzle(elem::s0, elem::s2);
  }

  __swizzled_vec__<DataType, 2> even() {
    return base_vec::swizzle(elem::s0, elem::s2);
  }
#include "triSYCL/vec/detail/swiz3.hpp"
};

template<typename DataType>
class alignas(detail::alignment_v<::trisycl::vec<DataType, 4>>)
  vec<DataType, 4> : public detail::vec<DataType, 4> {
  using base_vec = detail::vec<DataType, 4>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** Accessors to the first element of a vector
   */
  const DataType& x() const {
    return (*this)[0];
  }

  DataType& x() {
    return (*this)[0];
  }

  /** Accessors to the second element of a vector
   */
  const DataType& y() const {
    return (*this)[1];
  }

  DataType& y() {
    return (*this)[1];
  }

  /** Accessors to the third element of a vector
   */
  const DataType& z() const {
     return (*this)[2];
  }

  DataType& z() {
    return (*this)[2];
  }

  /** Accessors to the fourth element of a vector
   */
  const DataType& w() const {
     return (*this)[3];
  }

  DataType& w() {
    return (*this)[3];
  }

  /** Accessors to the first element of a vector
   */
  const DataType& r() const {
     return (*this)[0];
  }

  DataType& r() {
    return (*this)[0];
  }

  /** Accessors to the second element of a vector
   */
  const DataType& g() const {
      return (*this)[1];
  }

  DataType& g() {
    return (*this)[1];
  }

  /** Accessors to the third element of a vector
   */
  const DataType& b() const {
      return (*this)[2];
  }

  DataType& b() {
    return (*this)[2];
  }

  /** Accessors to the third element of a vector
   */
  const DataType& a() const {
      return (*this)[3];
  }

  DataType& a() {
    return (*this)[3];
  }

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
  TRISYCL_DECLARE_S(2);
  TRISYCL_DECLARE_S(3);

  const __swizzled_vec__<DataType, 2> lo() const {
    return base_vec::swizzle(elem::s0, elem::s1);
  }

  __swizzled_vec__<DataType, 2> lo()  {
    return base_vec::swizzle(elem::s0, elem::s1);
  }

  const __swizzled_vec__<DataType, 2> hi() const {
    return base_vec::swizzle(elem::s2, elem::s3);
  }

  __swizzled_vec__<DataType, 2> hi() {
    return base_vec::swizzle(elem::s2, elem::s3);
  }

  const __swizzled_vec__<DataType, 2> odd() const {
    return base_vec::swizzle(elem::s1, elem::s3);
  }

  __swizzled_vec__<DataType, 2> odd() {
    return base_vec::swizzle(elem::s1, elem::s3);
  }

  const __swizzled_vec__<DataType, 2> even() const {
    return base_vec::swizzle(elem::s0, elem::s2);
  }

  __swizzled_vec__<DataType, 2> even() {
    return base_vec::swizzle(elem::s0, elem::s2);
  }
#include "triSYCL/vec/detail/swiz4.hpp"
#include "triSYCL/vec/detail/swiz_rgba.hpp"
};

#undef TRISYCL_GEN_SWIZ2
#undef TRISYCL_GEN_SWIZ3
#undef TRISYCL_GEN_SWIZ4

template<typename DataType>
class alignas(detail::alignment_v<::trisycl::vec<DataType, 8>>)
  vec<DataType, 8> : public detail::vec<DataType, 8> {
  using base_vec = detail::vec<DataType, 8>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
  TRISYCL_DECLARE_S(2);
  TRISYCL_DECLARE_S(3);
  TRISYCL_DECLARE_S(4);
  TRISYCL_DECLARE_S(5);
  TRISYCL_DECLARE_S(6);
  TRISYCL_DECLARE_S(7);
  TRISYCL_DECLARE_S(8);

  const __swizzled_vec__<DataType, 4> lo() const {
    return base_vec::swizzle(elem::s0, elem::s1, elem::s2, elem::s3);
  }

  __swizzled_vec__<DataType, 4> lo() {
    return base_vec::swizzle(elem::s0, elem::s1, elem::s2, elem::s3);
  }

  const __swizzled_vec__<DataType, 4> hi() const {
    return base_vec::swizzle(elem::s4, elem::s5, elem::s6, elem::s7);
  }

  __swizzled_vec__<DataType, 4> hi() {
    return base_vec::swizzle(elem::s4, elem::s5, elem::s6, elem::s7);
  }

  const __swizzled_vec__<DataType, 4> odd() const {
    return base_vec::swizzle(elem::s1, elem::s3, elem::s5, elem::s7);
  }

  __swizzled_vec__<DataType, 4> odd() {
    return base_vec::swizzle(elem::s1, elem::s3, elem::s5, elem::s7);
  }

  const __swizzled_vec__<DataType, 4> even() const {
    return base_vec::swizzle(elem::s0, elem::s2, elem::s4, elem::s6);
  }

  __swizzled_vec__<DataType, 4> even() {
    return base_vec::swizzle(elem::s0, elem::s2, elem::s4, elem::s6);
  }
};


template<typename DataType>
class alignas(detail::alignment_v<::trisycl::vec<DataType, 16>>)
  vec<DataType, 16> : public detail::vec<DataType, 16> {
  using base_vec = detail::vec<DataType, 16>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
  TRISYCL_DECLARE_S(2);
  TRISYCL_DECLARE_S(3);
  TRISYCL_DECLARE_S(4);
  TRISYCL_DECLARE_S(5);
  TRISYCL_DECLARE_S(6);
  TRISYCL_DECLARE_S(7);
  TRISYCL_DECLARE_S(8);
  TRISYCL_DECLARE_S(9);
  TRISYCL_DECLARE_S(A);
  TRISYCL_DECLARE_S(B);
  TRISYCL_DECLARE_S(C);
  TRISYCL_DECLARE_S(D);
  TRISYCL_DECLARE_S(E);
  TRISYCL_DECLARE_S(F);

  const __swizzled_vec__<DataType, 8> lo() const {
    return base_vec::swizzle(elem::s0, elem::s1, elem::s2, elem::s3, elem::s4,
                             elem::s5, elem::s6, elem::s7);
  }

  __swizzled_vec__<DataType, 8> lo() {
    return base_vec::swizzle(elem::s0, elem::s1, elem::s2, elem::s3, elem::s4,
                             elem::s5, elem::s6, elem::s7);
  }

  const __swizzled_vec__<DataType, 8> hi() const {
    return base_vec::swizzle(elem::s8, elem::s9, elem::sA, elem::sB, elem::sC,
                             elem::sD, elem::sE, elem::sF);
  }

  __swizzled_vec__<DataType, 8> hi() {
    return base_vec::swizzle(elem::s8, elem::s9, elem::sA, elem::sB, elem::sC,
                             elem::sD, elem::sE, elem::sF);
  }

  const __swizzled_vec__<DataType, 8> odd() const {
    return base_vec::swizzle(elem::s1, elem::s3, elem::s5, elem::s7, elem::s9,
                             elem::sB, elem::sD, elem::sF);
  }

  __swizzled_vec__<DataType, 8> odd() {
    return base_vec::swizzle(elem::s1, elem::s3, elem::s5, elem::s7, elem::s9,
                             elem::sB, elem::sD, elem::sF);
  }

  const __swizzled_vec__<DataType, 8> even() const {
    return base_vec::swizzle(elem::s0, elem::s2, elem::s4, elem::s6, elem::s8,
                             elem::sA, elem::sC, elem::sE);
  }

  __swizzled_vec__<DataType, 8> even() {
    return base_vec::swizzle(elem::s0, elem::s2, elem::s4, elem::s6, elem::s8,
                             elem::sA, elem::sC, elem::sE);
  }

};

#undef TRISYCL_DECLARE_S

  /** A macro to define type alias, such as for type=uchar, size=4 and
      actual_type=unsigned char, uchar4 is equivalent to vec<unsigned char, 4>
  */
#define TRISYCL_DEFINE_VEC_TYPE_SIZE(type, size, actual_type) \
  using type##size = vec<actual_type, size>;

  /// Declare the vector types of a type for all the sizes
#define TRISYCL_DEFINE_VEC_TYPE(type, actual_type)           \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 1, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 2, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 3, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 4, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 8, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 16, actual_type)

  /// Declare all the possible vector type aliases
  TRISYCL_DEFINE_VEC_TYPE(char, char)
  TRISYCL_DEFINE_VEC_TYPE(uchar, unsigned char)
  TRISYCL_DEFINE_VEC_TYPE(short, short int)
  TRISYCL_DEFINE_VEC_TYPE(ushort, unsigned short int)
  TRISYCL_DEFINE_VEC_TYPE(int, int)
  TRISYCL_DEFINE_VEC_TYPE(uint, unsigned int)
  TRISYCL_DEFINE_VEC_TYPE(long, long int)
  TRISYCL_DEFINE_VEC_TYPE(ulong, unsigned long int)
  TRISYCL_DEFINE_VEC_TYPE(float, float)
  TRISYCL_DEFINE_VEC_TYPE(double, double)

/// @} End the vector Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VEC_HPP
