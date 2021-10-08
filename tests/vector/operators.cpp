/* RUN: %{execute}%s

   Test some vec<> behaviour with all the operators
*/
#include <CL/sycl.hpp>

#include <iostream>
#include <valarray>

#include <catch2/catch_test_macros.hpp>

using namespace cl::sycl;


/// Return true if both contents are the same
auto equal = [] (auto const &v, auto const &verif) {
  /* Do not use directly v == verif because we want to verify the
     (implicit) constructor itself */
  assert(v.size() == verif.size());
  auto p = std::begin(verif);
  for (auto e : v)
    if (e != *p++)
      return false;

  return true;
};

/** Use the std::valarray<> implementation to compare computations on
   vec<> and std::valarray<>
*/
#define TRISYCL_CHECK(OPERATOR, TYPE, SIZE, VAL1, VAL2) \
  {                                                     \
    std::initializer_list<TYPE> vil1 VAL1;              \
    std::initializer_list<TYPE> vil2 VAL2;              \
    using v = vec<TYPE, SIZE>;                          \
    using va = std::valarray<TYPE>;                     \
    v v1 VAL1;                                          \
    v v2 VAL2;                                          \
    v v3 = v1 OPERATOR v2;                              \
    va va1 { vil1 };                                    \
    va va2 { vil2 };                                    \
    va va3 = va1 OPERATOR va2;                          \
    REQUIRE(equal(v1, va1));                            \
    REQUIRE(equal(v2, va2));                            \
    REQUIRE(equal(v3, va3));                            \
    v3 OPERATOR##= v1;                                  \
    va3 OPERATOR##= va1;                                \
    REQUIRE(equal(v3, va3));                            \
  }


/** Use the std::valarray<> implementation to compare unary
    computations on vec<> and std::valarray<>
*/
#define TRISYCL_UNARY_CHECK(OPERATOR, TYPE, SIZE, VAL1) \
  {                                                     \
    std::initializer_list<TYPE> vil1 VAL1;              \
    using v = vec<TYPE, SIZE>;                          \
    using va = std::valarray<TYPE>;                     \
    v v1 VAL1;                                          \
    va va1 { vil1 };                                    \
    REQUIRE(equal(v1, va1));                            \
    v v2 = OPERATOR v1;                                 \
    va va2 = OPERATOR va1;                              \
    REQUIRE(equal(v1, va1));                            \
    REQUIRE(equal(v2, va2));                            \
  }

/** Use the std::valarray<> implementation to compare unary
    computations on vec<> and std::valarray<>

   The unary operator can be before or after the argument (like ++a or
   a++)
*/
#define TRISYCL_PREPOSTFIX_CHECK(BEFORE_OP, AFTER_OP, TYPE, SIZE, \
                           VAL, VAL_AFTER, RESULT)                \
  {                                                               \
    using v = vec<TYPE, SIZE>;                                    \
    v v1 VAL;                                                     \
    v v2 = BEFORE_OP v1 AFTER_OP;                                 \
    REQUIRE(equal(v1, v VAL_AFTER));                              \
    REQUIRE(equal(v2, v RESULT));                                 \
  }


TEST_CASE("operators", "[vector]") {
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
        vec<float, 4> v1 { 1, 2, 3, 4};
        vec<float, 4> v2 { 4, 5, 6, 7};
        auto v3 = v1 * v2;
        auto v4 = 1 + v3;
        v4 += v1;
        v4 += 3;
        std::valarray<float> va1 { 1, 2, 3, 4};
        std::valarray<float> va2 { 4, 5, 6, 7};
        /* The current valarray<> implementation does not work with
           auto va3 = va1 * va2; */
        std::valarray<float> va3 = va1 * va2;
        std::valarray<float> va4 = 1.F + va3;
        va4 += va1;
        va4 += 3.F;
        REQUIRE(equal(v1, va1));
        REQUIRE(equal(v2, va2));
        REQUIRE(equal(v3, va3));
        REQUIRE(equal(v4, va4));
        TRISYCL_CHECK(+, float, 4, ({ 1, 2, 3, 4}), ({ 4, 5, 6, 7}));
        TRISYCL_CHECK(-, float, 4, ({ 1, 2, 3, 4}), ({ 4, 5, 6, 7}));
        TRISYCL_CHECK(*, float, 4, ({ 1, 2, 3, 4}), ({ 4, 5, 6, 7}));
        TRISYCL_CHECK(/, float, 4, ({ 1, 2, 3, 4}), ({ 4, 5, 6, 7}));
        TRISYCL_CHECK(%, int, 4, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        TRISYCL_CHECK(&, char, 4, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        TRISYCL_CHECK(|, char, 4, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        TRISYCL_CHECK(^, int, 4, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        TRISYCL_CHECK(<<, int, 4, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        TRISYCL_CHECK(>>, int, 4, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        //TRISYCL_CHECK(&&, int, ({ 4, 5, 6, 7}), ({ 1, 2, 3, 4}));
        // +v
        TRISYCL_UNARY_CHECK(+, int, 4, ({ 1, 2, 3, 4}));
        // -v
        TRISYCL_UNARY_CHECK(-, int, 4, ({ 1, 2, 3, 4}));

        // std::valarray does not support -- and ++ :-(
        // ++v
        TRISYCL_PREPOSTFIX_CHECK(++,, int, 4, ({ 1, 2, 3, 4}),
                            ({ 2, 3, 4, 5}), ({ 2, 3, 4, 5}));

        // --v
        TRISYCL_PREPOSTFIX_CHECK(--,, int, 4, ({ 2, 3, 4, 5}),
                                 ({ 1, 2, 3, 4}), ({ 1, 2, 3, 4}));
        // v++
        TRISYCL_PREPOSTFIX_CHECK(,++, int, 4, ({ 1, 2, 3, 4}),
                                 ({ 2, 3, 4, 5}), ({ 1, 2, 3, 4}));
        // v--
        TRISYCL_PREPOSTFIX_CHECK(,--, int, 4, ({ 2, 3, 4, 5}),
                                 ({ 1, 2, 3, 4}), ({ 2, 3, 4, 5}));

                                       });
      }); // End of our commands for this queue
  } // End scope, so we wait for the queue to complete
}
