/* RUN: %{execute}%s

   Test the behaviour of math.cpp
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

#define TRYSYCL_MATH_WRAPX(FUN, i) std::cout << FUN(i) << std::endl;
#define TRYSYCL_MATH_WRAPX2(FUN, i) std::cout << FUN(i, i) << std::endl;
#define TRYSYCL_MATH_WRAPX2s(FUN, i) std::cout << FUN(i, i) << std::endl;
#define TRYSYCL_MATH_WRAPX3(FUN, i) std::cout << FUN(i, i, i) << std::endl;
#define TRYSYCL_MATH_WRAPX3s(FUN, i) std::cout << FUN(i, i, i) << std::endl;
#define TRYSYCL_MATH_WRAPX3ss(FUN, i) std::cout << FUN(i, i, i) << std::endl;

int test_main(int argc, char *argv[]) {
  constexpr auto N = 100;
  for (auto k = 0; k < N; k++) {
    double i = k;
    TRYSYCL_MATH_WRAPX(abs, i)//I
    //*TRYSYCL_MATH_WRAPX2(abs_diff)//I
    //*TRYSYCL_MATH_WRAPX2(add_sat)//I
    TRYSYCL_MATH_WRAPX(acos, i)
    TRYSYCL_MATH_WRAPX(acosh, i)
    //*TRYSYCL_MATH_WRAPX(acospi)
    TRYSYCL_MATH_WRAPX(asin, i)
    TRYSYCL_MATH_WRAPX(asinh, i)
    //*TRYSYCL_MATH_WRAPX(asinpi)
    TRYSYCL_MATH_WRAPX(atan, i) // atan(y/x)
    TRYSYCL_MATH_WRAPX2(atan2, i)
    TRYSYCL_MATH_WRAPX(atanh, i)
    //*TRYSYCL_MATH_WRAPX(atanpi)
    //*TRYSYCL_MATH_WRAPX2(atan2pi)
    TRYSYCL_MATH_WRAPX(cbrt, i)
    TRYSYCL_MATH_WRAPX(ceil, i)
    //*TRYSYCL_MATH_WRAPX3ss(clamp)//I
    //geninteger clamp(geninteger, sgeninteger, sgeninteger)
    //*TRYSYCL_MATH_WRAPX(clz)
    TRYSYCL_MATH_WRAPX2(copysign, i)
    TRYSYCL_MATH_WRAPX(cos, i)
    TRYSYCL_MATH_WRAPX(cosh, i)
    //*TRYSYCL_MATH_WRAPX(cospi)
    TRYSYCL_MATH_WRAPX(erfc, i)
    TRYSYCL_MATH_WRAPX(erf, i)
    TRYSYCL_MATH_WRAPX(exp, i)
    TRYSYCL_MATH_WRAPX(exp2, i)
    //*TRYSYCL_MATH_WRAPX(exp10)
    TRYSYCL_MATH_WRAPX(expm1, i)
    TRYSYCL_MATH_WRAPX(fabs, i)
    TRYSYCL_MATH_WRAPX2(fdim, i)
    TRYSYCL_MATH_WRAPX(floor, i)
    TRYSYCL_MATH_WRAPX3(fma, i)
    /* genfloat fmax ( genfloat x, genfloat y)
     * genfloat fmax ( genfloat x, sgenfloat y)
     */
    TRYSYCL_MATH_WRAPX2s(fmax, i)
    TRYSYCL_MATH_WRAPX2s(fmin, i)
    TRYSYCL_MATH_WRAPX2(fmod, i)
    //*TRYSYCL_MATH_WRAPX2s(fract)
    //**TRYSYCL_MATH_WRAPX2s(frexp)
    //*TRYSYCL_MATH_WRAPX(hadd)
    TRYSYCL_MATH_WRAPX2(hypot, i)
    //log
    //ilogb
    //ldexp
    TRYSYCL_MATH_WRAPX(lgamma, i)
    //*TRYSYCL_MATH_WRAPX2s(lgamma_r)
    TRYSYCL_MATH_WRAPX(log, i)
    TRYSYCL_MATH_WRAPX(log2, i)
    TRYSYCL_MATH_WRAPX(log10, i)
    TRYSYCL_MATH_WRAPX(log1p, i)
    TRYSYCL_MATH_WRAPX(logb, i)
    //*TRYSYCL_MATH_WRAPX3(mad)
    //*TRYSYCL_MATH_WRAPX3(mad_hi)//I
    //*TRYSYCL_MATH_WRAPX3(mad_sat)
    TRYSYCL_MATH_WRAPX3s(max, i) //I
    /* geninteger max (geninteger, geninteger)
     * geninteger max (geninteger, sgeninteger)
     */
    TRYSYCL_MATH_WRAPX2(max, i)
    //*TRYSYCL_MATH_WRAPX2(maxmag)
    TRYSYCL_MATH_WRAPX3s(min, i) //I
    /* geninteger min (geninteger, geninteger)
     * geninteger min (geninteger, sgeninteger)
     */
    TRYSYCL_MATH_WRAPX2(min, i)
    //*TRYSYCL_MATH_WRAPX2(minmag)
    //**TRYSYCL_MATH_WRAPX2s(modf)
    //*TRYSYCL_MATH_WRAPX2(mul_hi)//I
    //nan
    TRYSYCL_MATH_WRAPX2(pow, i)
    //*TRYSYCL_MATH_WRAPX2s(posn)
    //*TRYSYCL_MATH_WRAPX2(powr)
    TRYSYCL_MATH_WRAPX2(remainder, i)
    //**TRYSYCL_MATH_WRAPX3s(remquo)
    //*TRYSYCL_MATH_WRAPX(rhadd)//I
    TRYSYCL_MATH_WRAPX(rint, i)
    //*TRYSYCL_MATH_WRAPX3s(rootn)
    //**TRYSYCL_MATH_WRAPX2(rotate)//I
    TRYSYCL_MATH_WRAPX(round, i)
    //*TRYSYCL_MATH_WRAPX(rsqrt)
    TRYSYCL_MATH_WRAPX(sin, i)
    //*TRYSYCL_MATH_WRAPX2s(sincos)
    TRYSYCL_MATH_WRAPX(sinh, i)
    //*TRYSYCL_MATH_WRAPX(sinpi)
    TRYSYCL_MATH_WRAPX(sqrt, i)
    //*TRYSYCL_MATH_WRAPX2(sub_sat)
    TRYSYCL_MATH_WRAPX(tan, i)
    TRYSYCL_MATH_WRAPX(tanh, i)
    //*TRYSYCL_MATH_WRAPX(tanpi)
    TRYSYCL_MATH_WRAPX(tgamma, i)
    TRYSYCL_MATH_WRAPX(trunc, i)
    /*\ Integer concatenation
     * shortn upsample (charn hi, ucharn lo)
     * ushortn upsample (ucharn hi, ucharn lo)
     * intn upsample (shortn hi, ushortn lo)
     * uintn upsample (ushortn hi, ushortn lo)
     * longlongn upsample(intn hi, uintn lo)
     * ulonglongn upsample(uintn hi, uintn l)
    \*/
    //*TRYSYCL_MATH_WRAPX(popcount)//I
  }
  return 0;
}
