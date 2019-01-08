#ifndef TRISYCL_SYCL_MATH_HPP
#define TRISYCL_SYCL_MATH_HPP

/** \file
    Implement a wrapper around OpenCL math operations
    Joan.Thibault AT ens-rennes POINT fr
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <cmath>
#include <numeric>

#include "vec.hpp"

// Include order and configure insensitive treating of unwanted macros
#ifdef _MSC_VER
  #ifdef min
    #undef min
  #endif
  #ifdef max
    #undef max
  #endif
#endif

namespace cl {
namespace sycl {
#define TRISYCL_MATH_WRAP(FUN) template<typename T>                            \
  T FUN(T x) {                                                                 \
    return std::FUN(x);                                                        \
  }
#define TRISYCL_MATH_WRAP2(FUN) template<typename T>                           \
  T FUN(T x, T y) {                                                            \
    return std::FUN(x, y);                                                     \
  }
#define TRISYCL_MATH_WRAP2s(FUN) template<typename T, typename U>              \
  T FUN(T x, U y) {                                                            \
    return std::FUN(x, y);                                                     \
  }
#define TRISYCL_MATH_WRAP3(FUN) template<typename T>                           \
  T FUN(T x, T y, T z) {                                                       \
    return std::FUN(x, y, z);                                                  \
  }
#define TRISYCL_MATH_WRAP3s(FUN) template<typename T, typename U>              \
  T FUN(T x, T y, U z) {                                                       \
    return std::FUN(x, y, z);                                                  \
  }
#define TRISYCL_MATH_WRAP3ss(FUN) template<typename T, typename U>             \
  T FUN(T x, U y, U z) {                                                       \
    return std::FUN(x, y, z);                                                  \
  }

TRISYCL_MATH_WRAP(abs)//I
//*TRISYCL_MATH_WRAP2(abs_diff)//I
//*TRISYCL_MATH_WRAP2(add_sat)//I
TRISYCL_MATH_WRAP(acos)
TRISYCL_MATH_WRAP(acosh)
//*TRISYCL_MATH_WRAP(acospi)
TRISYCL_MATH_WRAP(asin)
TRISYCL_MATH_WRAP(asinh)
//*TRISYCL_MATH_WRAP(asinpi)
TRISYCL_MATH_WRAP(atan) // atan(y/x)
TRISYCL_MATH_WRAP2(atan2)
TRISYCL_MATH_WRAP(atanh)
//*TRISYCL_MATH_WRAP(atanpi)
//*TRISYCL_MATH_WRAP2(atan2pi)
TRISYCL_MATH_WRAP(cbrt)
TRISYCL_MATH_WRAP(ceil)
//*TRISYCL_MATH_WRAP3ss(clamp)//I
//geninteger clamp(geninteger, sgeninteger, sgeninteger)
//*TRISYCL_MATH_WRAP(clz)
TRISYCL_MATH_WRAP2(copysign)
TRISYCL_MATH_WRAP(cos)
TRISYCL_MATH_WRAP(cosh)
//*TRISYCL_MATH_WRAP(cospi)
TRISYCL_MATH_WRAP(erfc)
TRISYCL_MATH_WRAP(erf)
TRISYCL_MATH_WRAP(exp)
TRISYCL_MATH_WRAP(exp2)
//*TRISYCL_MATH_WRAP(exp10)
TRISYCL_MATH_WRAP(expm1)
TRISYCL_MATH_WRAP(fabs)
TRISYCL_MATH_WRAP2(fdim)
TRISYCL_MATH_WRAP(floor)
TRISYCL_MATH_WRAP3(fma)
/* genfloat fmax ( genfloat x, genfloat y)
 * genfloat fmax ( genfloat x, sgenfloat y)
 */
TRISYCL_MATH_WRAP2s(fmax)
TRISYCL_MATH_WRAP2s(fmin)
TRISYCL_MATH_WRAP2(fmod)
//*TRISYCL_MATH_WRAP2s(fract)
TRISYCL_MATH_WRAP2s(frexp)
//*TRISYCL_MATH_WRAP(hadd)
TRISYCL_MATH_WRAP2(hypot)
//log
//ilogb
//ldexp
TRISYCL_MATH_WRAP(lgamma)
//*TRISYCL_MATH_WRAP2s(lgamma_r)
TRISYCL_MATH_WRAP(log)
TRISYCL_MATH_WRAP(log2)
TRISYCL_MATH_WRAP(log10)
TRISYCL_MATH_WRAP(log1p)
TRISYCL_MATH_WRAP(logb)
//*TRISYCL_MATH_WRAP3(mad)
//*TRISYCL_MATH_WRAP3(mad_hi)//I
//*TRISYCL_MATH_WRAP3(mad_sat)
//
//TRISYCL_MATH_WRAP3s(max) //I
template<typename T>
T max(T x, T y, T z) {
  return std::max(x, std::max(y, z));
}
/* geninteger max (geninteger, geninteger)
 * geninteger max (geninteger, sgeninteger)
 */
  // Just import std::max from <algorithm>
  using std::max;

//*TRISYCL_MATH_WRAP2(maxmag)
//
//TRISYCL_MATH_WRAP3s(min) //I
template<typename T>
T min(T x, T y, T z) {
  return std::min(x, std::min(y, z));
}
/* geninteger min (geninteger, geninteger)
 * geninteger min (geninteger, sgeninteger)
 */
  // Just import std::min from <algorithm>
  using std::min;

//*TRISYCL_MATH_WRAP2(minmag)
TRISYCL_MATH_WRAP2s(modf)
//*TRISYCL_MATH_WRAP2(mul_hi)//I
//nan
TRISYCL_MATH_WRAP2(pow)
//*TRISYCL_MATH_WRAP2s(posn)
//*TRISYCL_MATH_WRAP2(powr)
TRISYCL_MATH_WRAP2(remainder)
TRISYCL_MATH_WRAP3s(remquo)
//*TRISYCL_MATH_WRAP(rhadd)//I
TRISYCL_MATH_WRAP(rint)
//*TRISYCL_MATH_WRAP3s(rootn)
TRISYCL_MATH_WRAP2(rotate)//I
TRISYCL_MATH_WRAP(round)
//*TRISYCL_MATH_WRAP(rsqrt)
TRISYCL_MATH_WRAP(sin)
//*TRISYCL_MATH_WRAP2s(sincos)
TRISYCL_MATH_WRAP(sinh)
//*TRISYCL_MATH_WRAP(sinpi)
TRISYCL_MATH_WRAP(sqrt)
//*TRISYCL_MATH_WRAP2(sub_sat)
TRISYCL_MATH_WRAP(tan)
TRISYCL_MATH_WRAP(tanh)
//*TRISYCL_MATH_WRAP(tanpi)
TRISYCL_MATH_WRAP(tgamma)
TRISYCL_MATH_WRAP(trunc)
/* Integer concatenation
 * shortn upsample (charn hi, ucharn lo)
 * ushortn upsample (ucharn hi, ucharn lo)
 * intn upsample (shortn hi, ushortn lo)
 * uintn upsample (ushortn hi, ushortn lo)
 * longlongn upsample(intn hi, uintn lo)
 * ulonglongn upsample(uintn hi, uintn l)
 */
//*TRISYCL_MATH_WRAP(popcount)//I
//*TRISYCL_MATH_WRAP3(mad24)
//*TRISYCL_MATH_WRAP3(mul24)

// Note: A lot of these math functions could be forwarded onto the OpenCL
// implementations when compiling for OpenCL

// Returns the cross product of p0.xyz and p1.xyz. The w component of float4
// result returned will be 0.0.
template <typename T, int size>
vec<T, size> cross(vec<T, size> p0,
                   vec<T, size> p1) {
  static_assert(!(size > 4 || size < 3), "vec type should be 3 or 4 dimensions");
  if constexpr (size > 3) {
    return vec<T, size> {
            (p0[1] * p1[2]) - (p0[2] * p1[1]),
            (p0[2] * p1[0]) - (p0[0] * p1[2]),
            (p0[0] * p1[1]) - (p0[1] * p1[0]),
            0.0};
  } else {
    return vec<T, size> {
            (p0[1] * p1[2]) - (p0[2] * p1[1]),
            (p0[2] * p1[0]) - (p0[0] * p1[2]),
            (p0[0] * p1[1]) - (p0[1] * p1[0])};
  }
}

// Return the length of vector p, i.e., sqrt(p.x^2 + p.y^2 + ...)
template <typename T, int size>
T length(vec<T, size> p) {
  auto temp = p * p;
  return std::sqrt(std::accumulate(temp.begin(), temp.end(), 0.0));
}

// Compute dot product.
template <typename T, int size>
T dot(vec<T, size> p0, vec<T, size> p1) {
  return std::inner_product(p0.begin(), p0.end(), p1.begin(), T{0});
}

// Returns fmin(fmax(x, minval), maxval). Results are undefined if
// minval > maxval.
// \todo Currently this clamp requires all values to be derived from the same
// type T. A better implementation would be to make the vector clamp a
// specialization of the 1d clamp and perhaps use a Using clamp = clamp_impl
// type interface. Alternatively restriction from the more generic template
// could be done using a type_trait and a typename T2 could be added
template <typename T>
T clamp(T x, T minval, T maxval) {
  return std::fmin(std::fmax(x, minval), maxval);
}

template <typename T, int size>
vec<T, size> clamp(vec<T, size> x, T minval, T maxval) {
  vec<T, size> temp {0};
  std::transform(x.begin(), x.end(), temp.begin(),[minval, maxval](auto a) {
                                return clamp(a, minval, maxval);
                             });
  return temp;
}

// Returns a vector in the same direction as p but with a length of 1.
template <typename T, int size>
vec<T, size> normalize(vec<T, size> p) {
  return p / length(p);
}

// Round to integral value using the round to
// negative infinity rounding mode.
template <typename T, int size>
vec<T, size> floor(vec<T, size> x) {
  vec<T, size> temp;
  std::transform(x.begin(), x.end(), temp.begin(), [](auto a) { return std::floor(a); });
  return temp;
}

// Returns y if y < x, otherwise it returns x.
// genfloat fmin ( genfloat x, sgenfloat y )
// If one argument is a NaN, fmin() returns
// the other argument. If both arguments are
// NaNs, fmin() returns a NaN.
//
// \todo Perhaps worth adding an isNaN check as I'm not sure if the < operator
//       conforms to what the function requires
// \todo Only defined for vec's should be defined for floats/doubles
template <typename T, int size>
vec<T, size> fmin(vec<T, size> x,
                  vec<T, size> y) {
 vec<T, size> temp;
 for(int i = 0; i < size; ++i)
 temp[i] = std::fmin(y[i],x[i]);
 return temp;
}

// Returns y if x < y, otherwise it returns x.
// If one argument is a NaN, fmax() returns
// the other argument. If both arguments are
// NaNs, fmax() returns a NaN
template <typename T, int size>
vec<T, size> fmax(vec<T, size> x,
                  vec<T, size> y) {
   vec<T, size> temp;
   for(int i = 0; i < size; ++i)
   temp[i] = std::fmax(x[i],y[i]);
   return temp;
}

// seems to essentially be the same as fmax but cover different types, even the
// first type definition matches fmax's first? Is there a need for two
// similar definitions of max? same goes for fmin...
// genfloat max (genfloat x, genfloat y)
// genfloatf max (genfloatf x, float y)
// genfloatd max (genfloatd x, double y)
// geninteger max (geninteger x, geninteger y)
// geninteger max (geninteger x, sgeninteger y)
template <typename T, int size>
vec<T, size> max(vec<T, size> x,
                  vec<T, size> y) {
   return fmax(x,y);
}

template <typename T, int size>
vec<T, size> min(vec<T, size> x,
                 vec<T, size> y) {
   return fmin(x,y);
}

//
namespace native {
TRISYCL_MATH_WRAP(cos)
//*TRISYCL_MATH_WRAP2(divide)
TRISYCL_MATH_WRAP(exp)
TRISYCL_MATH_WRAP(exp2)
//*TRISYCL_MATH_WRAP(exp10)
TRISYCL_MATH_WRAP(log)
TRISYCL_MATH_WRAP(log2)
TRISYCL_MATH_WRAP(log10)
//*TRISYCL_MATH_WRAP(powr)
//*TRISYCL_MATH_WRAP(recip)
//*TRISYCL_MATH_WRAP(rsqrt)
TRISYCL_MATH_WRAP(sin)
TRISYCL_MATH_WRAP(sqrt)
TRISYCL_MATH_WRAP(tan)
}
#undef TRISYCL_MATH_WRAP
#undef TRISYCL_MATH_WRAP2
#undef TRISYCL_MATH_WRAP2s
#undef TRISYCL_MATH_WRAP3
#undef TRISYCL_MATH_WRAP3s
#undef TRISYCL_MATH_WRAP3ss

}
}

#endif
