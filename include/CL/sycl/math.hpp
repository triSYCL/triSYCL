#ifndef TRISYCL_SYCL_MATH_HPP
#define TRISYCL_SYCL_MATH_HPP

/** \file
    Implement a wrapper to OpenCL math operations
    Joan.Thibault AT ens-rennes POINT fr
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cmath>

namespace cl {
namespace sycl {
  #define wrap(FUN) template<typename T> T FUN(T x) {return std::FUN(x);}
  #define wrap2(FUN) template<typename T> T FUN(T x, T y) {return std::FUN(x, y);}
  #define wrap2s(FUN) template<typename T, typename U> T FUN(T x, U y) {return std::FUN(x, y);}
  #define wrap3(FUN) template<typename T> T FUN(T x, T y, T z) {return std::FUN(x, y, z);}
  #define wrap3s(FUN) template<typename T, typename U> T FUN(T x, T y, U z) {return std::FUN(x, y, z);}
  #define wrap3ss(FUN) template<typename T, typename U> T FUN(T x, U y, U z) {return std::FUN(x, y, z);}

  wrap(abs)//I
  //*wrap2(abs_diff)//I
  //*wrap2(add_sat)//I
  wrap(acos)
  wrap(acosh)
  //*wrap(acospi)
  wrap(asin)
  wrap(asinh)
  //*wrap(asinpi)
  wrap(atan) // atan(y/x)
  wrap2(atan2)
  wrap(atanh)
  //*wrap(atanpi)
  //*wrap2(atan2pi)
  wrap(cbrt)
  wrap(ceil)
  //*wrap3ss(clamp)//I
  //geninteger clamp(geninteger, sgeninteger, sgeninteger)
  //*wrap(clz)
  wrap2(copysign)
  wrap(cos)
  wrap(cosh)
  //*wrap(cospi)
  wrap(erfc)
  wrap(erf)
  wrap(exp)
  wrap(exp2)
  //*wrap(exp10)
  wrap(expm1)
  wrap(fabs)
  wrap2(fdim)
  wrap(floor)
  wrap3(fma)
  /* genfloat fmax ( genfloat x, genfloat y)
   * genfloat fmax ( genfloat x, sgenfloat y)
   */
  wrap2s(fmax)
  wrap2s(fmin)
  wrap2(fmod)
  //*wrap2s(fract)
  wrap2s(frexp)
  //*wrap(hadd)
  wrap2(hypot)
  //log
  //ilogb
  //ldexp
  wrap(lgamma)
  //*wrap2s(lgamma_r)
  wrap(log)
  wrap(log2)
  wrap(log10)
  wrap(log1p)
  wrap(logb)
  //*wrap3(mad)
  //*wrap3(mad_hi)//I
  //*wrap3(mad_sat)
  //
  //wrap3s(max) //I
  template<typename T>
  T max(T x, T y, T z){
    return std::max(x, std::max(y, z));
  }
  /* geninteger max (geninteger, geninteger)
   * geninteger max (geninteger, sgeninteger)
   */

  //*wrap2(maxmag)
  //
  //wrap3s(min) //I
  template<typename T>
  T min(T x, T y, T z){
    return std::min(x, std::min(y, z));
  }
  /* geninteger min (geninteger, geninteger)
   * geninteger min (geninteger, sgeninteger)
   */

  //*wrap2(minmag)
  wrap2s(modf)
  //*wrap2(mul_hi)//I
  //nan
  wrap2(pow)
  //*wrap2s(posn)
  //*wrap2(powr)
  wrap2(remainder)
  wrap3s(remquo)
  //*wrap(rhadd)//I
  wrap(rint)
  //*wrap3s(rootn)
  wrap2(rotate)//I
  wrap(round)
  //*wrap(rsqrt)
  wrap(sin)
  //*wrap2s(sincos)
  wrap(sinh)
  //*wrap(sinpi)
  wrap(sqrt)
  //*wrap2(sub_sat)
  wrap(tan)
  wrap(tanh)
  //*wrap(tanpi)
  wrap(tgamma)
  wrap(trunc)
  /*\ Integer concatenation
   * shortn upsample (charn hi, ucharn lo)
   * ushortn upsample (ucharn hi, ucharn lo)
   * intn upsample (shortn hi, ushortn lo)
   * uintn upsample (ushortn hi, ushortn lo)
   * longlongn upsample(intn hi, uintn lo)
   * ulonglongn upsample(uintn hi, uintn l)
  \*/
  //*wrap(popcount)//I
  //*wrap3(mad24)
  //*wrap3(mul24)

  //
  namespace native
  {
    wrap(cos)
    //*wrap2(divide)
    wrap(exp)
    wrap(exp2)
    //*wrap(exp10)
    wrap(log)
    wrap(log2)
    wrap(log10)
    //*wrap(powr)
    //*wrap(recip)
    //*wrap(rsqrt)
    wrap(sin)
    wrap(sqrt)
    wrap(tan)
  }

}
}

#endif
