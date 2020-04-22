/* RUN: %{execute}%s

   Test some vec<> behaviour
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <boost/test/minimal.hpp>

using namespace cl::sycl;


/// Return true if both contents are the same
template <typename Vec>
bool equal(const Vec &v, const Vec &verif) {
  /* Do not use directly v == verif because we want to verify the
     (implicit) constructor itself */
  auto p = std::begin(verif);
  for (auto e : v)
    if (e != *p++)
      return false;

  return true;
}




int test_main(int argc, char *argv[]) {

  constexpr size_t N = 16;
  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    queue myQueue;

    // A buffer of N float using the storage of c
    buffer<float> C(N);

    /* The command group describing all operations needed for the kernel
       execution */
    myQueue.submit([&](handler &cgh) {
      auto kc = C.get_access<access::mode::write>(cgh);

      cgh.parallel_for<class generate>(range<1> { N },
                                       [=] (id<1> index) {
        vec<float, 16> v;
        float1 v1 = 1.5F;
        BOOST_CHECK(equal(v1, { 1.5F }));
        auto v1_1 = v1 + v1;
        BOOST_CHECK(v1_1[0] == 3.F);
        float2 v2 = { 6, 3};
        BOOST_CHECK(equal(v2, { 6, 3 }));
        float2 v2_c(4, 3);
        BOOST_CHECK(equal(v2_c, { 4, 3 }));
        float2 v2_v1 = { v1, v1 };
        BOOST_CHECK(equal(v2_v1, { 1.5, 1.5 }));
        double2 v2_d = v2/v2_c;
        BOOST_CHECK(equal(v2_d, { 1.5, 1 }));
        float2 v2f = { 2.F, 3.6F };
        BOOST_CHECK(equal(v2f, { 2, 3.6 }));
        v2f += 1;
        BOOST_CHECK(equal(v2f, { 3, 4.6 }));
        float4 v4 = { 2.F, 3.6F, 3, -8 };
        BOOST_CHECK(equal(v4, { 2, 3.6, 3, -8 }));
        float4 v4_v2_v1 = { 2.F, v2, v1 };
        BOOST_CHECK(equal(v4_v2_v1, { 2, 6, 3, 1.5 }));
        v4 = v4_v2_v1*3;
        BOOST_CHECK(equal(v4, { 6, 18, 9, 4.5 }));
        float16 v16_broadcast = 2.6;
        BOOST_CHECK(equal(v16_broadcast, {
              2.6, 2.6, 2.6, 2.6,
                2.6, 2.6, 2.6, 2.6,
                2.6, 2.6, 2.6, 2.6,
                2.6, 2.6, 2.6, 2.6,
                }));
        v = 0;
        BOOST_CHECK(equal(v16_broadcast + v, v16_broadcast));
        uchar2 uc2 = 3;
        BOOST_CHECK(equal(uc2, { '\003', 3 }));
        char3 c3(1, uc2);
        BOOST_CHECK(equal(c3, { 1, '\003', 03 }));
        auto c3_identity = c3 & c3;
        BOOST_CHECK(equal(c3_identity, c3));
        // There is a truncation on v1
        short4 s4 = { c3, v1 };
        BOOST_CHECK(equal(s4, { 1, '\003', 03, 1 }));
        ulong8 ul8(s4, 1, 2, uc2);
        BOOST_CHECK(equal(ul8, { 1, '\003', 03, 1, 1, 2, 3, 3 }));
        double16 d16(3.14, 2.718, 1.602, ul8, c3, v2);
        BOOST_CHECK(equal(d16, {
              3.14, 2.718, 1.602, 1,
                3, 3, 1, 1,
                2, 3, 3, 1,
                3, 3, 6, 3
                }));
        // d16.display();
        auto d16_2 = d16 + d16;
        BOOST_CHECK(equal(d16_2, {
              6.28, 5.436, 3.204, 2,
                6, 6, 2, 2,
                4, 6, 6, 2,
                6, 6, 12, 6
                }));
                                       });
      }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete

  return 0;
}
