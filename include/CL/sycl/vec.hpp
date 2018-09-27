#ifndef TRISYCL_SYCL_VEC_HPP
#define TRISYCL_SYCL_VEC_HPP

/** \file

    Implement the small OpenCL vector class

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl::sycl {

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
}

/** \addtogroup vector Vector types in SYCL

    @{
*/


/** Small OpenCL vector class
*/


#include "CL/sycl/vec/detail/vec.hpp"

namespace cl::sycl {
  /** Accessors to access hex indexed elements of a vector
   * There are two macros, one for 0-9, one for A-F.
   */
#define TRISYCL_DECLARE_S(x)                                            \
  DataType& s##x() {                                                    \
    return (*this)[(x)];                                                \
  }

#define TRISYCL_DECLARE_Sx(x)                                           \
  DataType& s##x() {                                                    \
    return (*this)[(0x##x)];                                            \
  }

template<typename DataType>
class vec<DataType, 1> : public detail::vec<DataType, 1> {
  using base_vec = detail::vec<DataType, 1>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** An accessor to the first element of a vector
   */
  DataType& x(){
    return (*this)[0];
  }

  TRISYCL_DECLARE_S(0);

  operator DataType() const {
    return (*this)[0];
  }
};

template<typename DataType>
class vec<DataType, 2> : public detail::vec<DataType, 2> {
  using base_vec = detail::vec<DataType, 2>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** An accessor to the first element of a vector
   */
  DataType& x(){
    return (*this)[0];
  }

    /** An accessor to the second element of a vector
   */
  DataType& y(){
    return (*this)[1];
  }

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
};

template<typename DataType>
class vec<DataType, 3> : public detail::vec<DataType, 3> {
  using base_vec = detail::vec<DataType, 3>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** An accessor to the first element of a vector
   */
  DataType& x(){
    return (*this)[0];
  }

  /** An accessor to the second element of a vector
   */
  DataType& y(){
    return (*this)[1];
  }

  /** An accessor to the second element of a vector
   */
  DataType& z(){
    return (*this)[2];
  }

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
  TRISYCL_DECLARE_S(2);
};

template<typename DataType>
class vec<DataType, 4> : public detail::vec<DataType, 4> {
  using base_vec = detail::vec<DataType, 4>;

public:

  /* use base class constructors */
  using base_vec::base_vec;

  /** An accessor to the first element of a vector
   */
  DataType& x(){
    return (*this)[0];
  }

  /** An accessor to the second element of a vector
   */
  DataType& y(){
    return (*this)[1];
  }

  /** An accessor to the second element of a vector
   */
  DataType& z(){
    return (*this)[2];
  }

  /** An accessor to the second element of a vector
   */
  DataType& w(){
    return (*this)[3];
  }

  /** An accessor to the first element of a vector
   */
  DataType& r(){
    return (*this)[0];
  }

  /** An accessor to the second element of a vector
   */
  DataType& g(){
    return (*this)[1];
  }

  /** An accessor to the third element of a vector
   */
  DataType& b(){
    return (*this)[2];
  }

  /** An accessor to the fourth element of a vector
   */
  DataType& a(){
    return (*this)[3];
  }

  TRISYCL_DECLARE_S(0);
  TRISYCL_DECLARE_S(1);
  TRISYCL_DECLARE_S(2);
  TRISYCL_DECLARE_S(3);
};

template<typename DataType>
class vec<DataType, 8> : public detail::vec<DataType, 8> {
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
};


template<typename DataType>
class vec<DataType, 16> : public detail::vec<DataType, 16> {
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
  TRISYCL_DECLARE_Sx(A);
  TRISYCL_DECLARE_Sx(B);
  TRISYCL_DECLARE_Sx(C);
  TRISYCL_DECLARE_Sx(D);
  TRISYCL_DECLARE_Sx(E);
  TRISYCL_DECLARE_Sx(F);
};

#undef TRISYCL_DECLARE_S
#undef TRISYCL_DECLARE_Sx

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
