/* RUN: %{execute}%s

   This is just a clone of vector/vec.cpp for now.
*/
#include <sycl/sycl.hpp>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

/// Return true if both contents are the same
template <typename Marray> bool equal(const Marray& v, const Marray& verif) {
  /* Do not use directly v == verif because we want to verify the
     (implicit) constructor itself */
  auto p = std::begin(verif);
  for (auto e : v)
    if (e != *p++)
      return false;

  return true;
}

TEST_CASE("construction and basic operations", "[marray]") {
  constexpr size_t n = 16;
  { // By sticking all the SYCL work in a {} block, we ensure
    // all SYCL tasks must complete before exiting the block

    // Create a queue to work on
    sycl::queue q;

    // A buffer of N float using the storage of c
    sycl::buffer<float> c(n);

    /* The command group describing all operations needed for the kernel
       execution */
    q.submit([&](sycl::handler& cgh) {
      auto kc = c.get_access<sycl::access::mode::write>(cgh);

      cgh.parallel_for(sycl::range<1> { n }, [=](sycl::id<1> index) {
        sycl::marray<float, 16> v;
        sycl::mfloat1 v1 = 1.5F;
        REQUIRE(equal(v1, { 1.5F }));
        auto v1_1 = v1 + v1;
        REQUIRE(v1_1[0] == 3.F);
        sycl::mfloat2 v2 = { 6, 3 };
        REQUIRE(equal(v2, { 6, 3 }));
        sycl::mfloat2 v2_c(4, 3);
        REQUIRE(equal(v2_c, { 4, 3 }));
        sycl::mfloat2 v2_v1 = { v1, v1 };
        REQUIRE(equal(v2_v1, { 1.5, 1.5 }));
        sycl::mdouble2 v2_d = v2 / v2_c;
        REQUIRE(equal(v2_d, { 1.5, 1 }));
        sycl::mfloat2 v2f = { 2.F, 3.6F };
        REQUIRE(equal(v2f, { 2, 3.6 }));
        v2f += 1;
        REQUIRE(equal(v2f, { 3, 4.6 }));
        sycl::mfloat4 v4 = { 2.F, 3.6F, 3, -8 };
        REQUIRE(equal(v4, { 2, 3.6, 3, -8 }));
        sycl::mfloat4 v4_v2_v1 = { 2.F, v2, v1 };
        REQUIRE(equal(v4_v2_v1, { 2, 6, 3, 1.5 }));
        v4 = v4_v2_v1 * 3;
        REQUIRE(equal(v4, { 6, 18, 9, 4.5 }));
        sycl::mfloat16 v16_broadcast = 2.6;
        REQUIRE(equal(v16_broadcast, {
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                         2.6,
                                     }));
        v = 0;
        REQUIRE(equal(v16_broadcast + v, v16_broadcast));
        sycl::muchar2 uc2 = 3;
        REQUIRE(equal(uc2, { '\003', 3 }));
        sycl::mchar3 c3(1, uc2);
        REQUIRE(equal(c3, { 1, '\003', 03 }));
        auto c3_identity = c3 & c3;
        REQUIRE(equal(c3_identity, c3));
        // There is a truncation on 1.5
        sycl::mshort4 s4 = { c3, v1 };
        REQUIRE(equal(s4, { 1, '\003', 03, 1 }));
        sycl::mulong8 ul8(s4, 1, 2, uc2);
        REQUIRE(equal(ul8, { 1, '\003', 03, 1, 1, 2, 3, 3 }));
        sycl::mdouble16 d16(3.14, 2.718, 1.602, ul8, c3, v2);
        REQUIRE(equal(d16, { 3.14, 2.718, 1.602, 1, 3, 3, 1, 1, 2, 3, 3, 1, 3,
                             3, 6, 3 }));
        // d16.display();
        auto d16_2 = d16 + d16;
        REQUIRE(equal(d16_2, { 6.28, 5.436, 3.204, 2, 6, 6, 2, 2, 4, 6, 6, 2, 6,
                               6, 12, 6 }));
      });
    }); // End of our commands for this queue
  }     // End scope, so we wait for the queue to complete
}
