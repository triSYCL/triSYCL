#ifndef TRISYCL_SYCL_MATH_HPP
#define TRISYCL_SYCL_MATH_HPP

/** \file
    Implement a wrapper around OpenCL math operations
    Joan.Thibault AT ens-rennes POINT fr
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cmath>

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
