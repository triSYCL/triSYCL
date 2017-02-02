/* RUN: %{execute}%s

   Test some vec<> behaviour
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;

#define wrapX(FUN, i) std::cout << FUN(i) << std::endl;
#define wrapX2(FUN, i) FUN(i, i);
#define wrapX2s(FUN, i) FUN(i, i);
#define wrapX3(FUN, i) FUN(i, i, i);
#define wrapX3s(FUN, i) FUN(i, i, i);
#define wrapX3ss(FUN, i) FUN(i, i, i);

int test_main(int argc, char *argv[]) {
  constexpr auto N = 100;
  for(int k = 0; k < N; k++)
  {
    double i = k;
    wrapX(abs, i)//I
    //*wrapX2(abs_diff)//I
    //*wrapX2(add_sat)//I
    wrapX(acos, i)
    wrapX(acosh, i)
    //*wrapX(acospi)
    wrapX(asin, i)
    wrapX(asinh, i)
    //*wrapX(asinpi)
    wrapX(atan, i) // atan(y/x)
    wrapX2(atan2, i)
    wrapX(atanh, i)
    //*wrapX(atanpi)
    //*wrapX2(atan2pi)
    wrapX(cbrt, i)
    wrapX(ceil, i)
    //*wrapX3ss(clamp)//I
    //geninteger clamp(geninteger, sgeninteger, sgeninteger)
    //*wrapX(clz)
    wrapX2(copysign, i)
    wrapX(cos, i)
    wrapX(cosh, i)
    //*wrapX(cospi)
    wrapX(erfc, i)
    wrapX(erf, i)
    wrapX(exp, i)
    wrapX(exp2, i)
    //*wrapX(exp10)
    wrapX(expm1, i)
    wrapX(fabs, i)
    wrapX2(fdim, i)
    wrapX(floor, i)
    wrapX3(fma, i)
    /* genfloat fmax ( genfloat x, genfloat y)
     * genfloat fmax ( genfloat x, sgenfloat y)
     */
    wrapX2s(fmax, i)
    wrapX2s(fmin, i)
    wrapX2(fmod, i)
    //*wrapX2s(fract)
    //**wrapX2s(frexp)
    //*wrapX(hadd)
    wrapX2(hypot, i)
    //log
    //ilogb
    //ldexp
    wrapX(lgamma, i)
    //*wrapX2s(lgamma_r)
    wrapX(log, i)
    wrapX(log2, i)
    wrapX(log10, i)
    wrapX(log1p, i)
    wrapX(logb, i)
    //*wrapX3(mad)
    //*wrapX3(mad_hi)//I
    //*wrapX3(mad_sat)
    wrapX3s(max, i) //I
    /* geninteger max (geninteger, geninteger)
     * geninteger max (geninteger, sgeninteger)
     */
    //*wrapX2(maxmag)
    wrapX3s(min, i) //I
    /* geninteger min (geninteger, geninteger)
     * geninteger min (geninteger, sgeninteger)
     */
    //*wrapX2(minmag)
    //**wrapX2s(modf)
    //*wrapX2(mul_hi)//I
    //nan
    wrapX2(pow, i)
    //*wrapX2s(posn)
    //*wrapX2(powr)
    wrapX2(remainder, i)
    //**wrapX3s(remquo)
    //*wrapX(rhadd)//I
    wrapX(rint, i)
    //*wrapX3s(rootn)
    //**wrapX2(rotate)//I
    wrapX(round, i)
    //*wrapX(rsqrt)
    wrapX(sin, i)
    //*wrapX2s(sincos)
    wrapX(sinh, i)
    //*wrapX(sinpi)
    wrapX(sqrt, i)
    //*wrapX2(sub_sat)
    wrapX(tan, i)
    wrapX(tanh, i)
    //*wrapX(tanpi)
    wrapX(tgamma, i)
    wrapX(trunc, i)
    /*\ Integer concatenation
     * shortn upsample (charn hi, ucharn lo)
     * ushortn upsample (ucharn hi, ucharn lo)
     * intn upsample (shortn hi, ushortn lo)
     * uintn upsample (ushortn hi, ushortn lo)
     * longlongn upsample(intn hi, uintn lo)
     * ulonglongn upsample(uintn hi, uintn l)
    \*/
    //*wrapX(popcount)//I
  }
  return 0;
}
