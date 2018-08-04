/* RUN: %{execute}%s

   Test the operators defined for the different cl_types
   When not running in OpenCL interoperability mode
*/

#include <CL/sycl.hpp>
#include <iostream>
#include <valarray>
#include <boost/test/minimal.hpp>

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

/* Accessors for the type tuples (name, cl_name, is_float) */
#define TYPE_NAME(T)      BOOST_PP_TUPLE_ELEM(3, 0, T)
#define TYPE_CL_NAME(T)   BOOST_PP_TUPLE_ELEM(3, 1, T)
#define TYPE_IS_FLOAT(T)  BOOST_PP_TUPLE_ELEM(3, 2, T)

/* To generate the sized types (eg. \c cl_int3) */
#define SIZED_NAME(T, size)                                                    \
  BOOST_PP_IF(                                                                 \
    BOOST_PP_EQUAL(size, 1), cl::sycl::T,                                      \
    BOOST_PP_CAT(cl::sycl::T, size))

/* Sequence of defined sizes for \c cl_types */
#define SIZES (1)(2)(3)(4)(8)(16)

/* Accessors for the operator datatype. */
#define OP_SYMBOL(O)      BOOST_PP_TUPLE_ELEM(5, 0, O)
#define OP_NAME(O)        BOOST_PP_TUPLE_ELEM(5, 1, O)
#define OP_IS_FLOATING(O) BOOST_PP_TUPLE_ELEM(5, 2, O)
#define OP_IS_LOGICAL(O)  BOOST_PP_TUPLE_ELEM(5, 3, O)
#define OP_IS_SHIFTING(O) BOOST_PP_TUPLE_ELEM(5, 4, O)

/* List of implemented operators */
#define BINARY_OPS                                                             \
  BOOST_PP_TUPLE_TO_LIST(                                                      \
    18,                                                                        \
    (                                                                          \
      ( *  , mul           ,1 ,0 ,0),                                          \
      ( /  , div           ,1 ,0 ,0),                                          \
      ( %  , mod           ,0 ,0 ,0),                                          \
      ( +  , add           ,1 ,0 ,0),                                          \
      ( -  , sub           ,1 ,0 ,0),                                          \
      ( << , shift_left    ,0 ,0 ,1),                                          \
      ( >> , shift_right   ,0 ,0 ,1),                                          \
      ( <  , less          ,1 ,1 ,0),                                          \
      ( <= , less_equal    ,1 ,1 ,0),                                          \
      ( >= , greater_equal ,1 ,1 ,0),                                          \
      ( >  , greater       ,1 ,1 ,0),                                          \
      ( == , equal         ,1 ,1 ,0),                                          \
      ( != , not_equal     ,1 ,1 ,0),                                          \
      ( &  , bitwise_and   ,0 ,0 ,0),                                          \
      ( |  , bitwise_or    ,0 ,0 ,0),                                          \
      ( ^  , bitwise_xor   ,0 ,0 ,0),                                          \
      ( && , logical_and   ,1 ,1 ,0),                                          \
      ( || , logical_or    ,1 ,1 ,0)                                           \
      )                                                                        \
    )

/* Test an operator by comparing the result of the same operation
   on a \c valarray and on a \c cl_type
*/
#define TRISYCL_CHECK(OPERATOR, TYPE, CL_TYPE, SIZE, VAL1, VAL2, OUT)          \
  {                                                                            \
    std::initializer_list<TYPE> vil1 (VAL1);                                   \
    std::initializer_list<TYPE> vil2 (VAL2);                                   \
    using v = BOOST_PP_IF(BOOST_PP_EQUAL(SIZE, 1),                             \
                          BOOST_PP_CAT(cl::sycl::CL_TYPE, SIZE),               \
                          BOOST_PP_CAT(                                        \
                            BOOST_PP_CAT(cl::sycl::cl_, CL_TYPE), SIZE));      \
    using va_in = std::valarray<TYPE>;                                         \
    using va_out = std::valarray<OUT>;                                         \
    v v1 (VAL1);                                                               \
    v v2 (VAL2);                                                               \
    v v3 = v2 OPERATOR v1;                                                     \
    va_in va1 { vil1 };                                                        \
    va_in va2 { vil2 };                                                        \
    va_out va3 = va1 OPERATOR va2;                                             \
    BOOST_CHECK(equal(v1, v2));                                                \
    BOOST_CHECK(equal(v1, va1));                                               \
    BOOST_CHECK(equal(v2, va2));                                               \
    BOOST_CHECK(equal(v3, va3));                                               \
  }

/* The values for the operation
   \todo remplace this by vector generator
*/
#define VAL_1 { 1 }

#define VAL_2 { 1, 2 }

#define VAL_3 { 1, 2, 3 }

#define VAL_4 { 1, 2, 3, 4 }

#define VAL_8 { 1, 2, 3, 4, 5, 6, 7, 8 }

#define VAL_16 { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }

// Accessor for the tuple \c (type, size, is_float)
#define ACCESS_TUPLE_TYPE(T)  BOOST_PP_TUPLE_ELEM(3, 0, T)
#define ACCESS_TUPLE_SIZE(T)  BOOST_PP_TUPLE_ELEM(3, 1, T)

// Does nothing
#define EMPTY(a, b, c) BOOST_PP_EMPTY()

// Wrapper around \c TRISYCL_CHECK
#define CALL_CHECK(op, data, out)                                              \
  TRISYCL_CHECK(OP_SYMBOL(op), TYPE_NAME(ACCESS_TUPLE_TYPE(data)),             \
                TYPE_CL_NAME(ACCESS_TUPLE_TYPE(data)),                         \
                ACCESS_TUPLE_SIZE(data),                                       \
                BOOST_PP_CAT(VAL_, ACCESS_TUPLE_SIZE(data)),                   \
                BOOST_PP_CAT(VAL_, ACCESS_TUPLE_SIZE(data)), out);

// Determine if the \c valarray with the result will be bool or not
#define GET_OUT_TYPE(op, type)                                                 \
  BOOST_PP_IF(                                                                 \
    OP_IS_LOGICAL(op),                                                         \
    bool,                                                                      \
    TYPE_NAME(type))

// Check if the operation is compatible with float types
#define VERIF_FLOAT(op, data, out)                                             \
  BOOST_PP_IF(                                                                 \
    BOOST_PP_OR(OP_IS_FLOATING(op),                                            \
                BOOST_PP_NOT(TYPE_IS_FLOAT(ACCESS_TUPLE_TYPE(data)))),         \
    CALL_CHECK, EMPTY)(op, data, out)

// Wrapper around \c VERIF_FLOAT
#define GENERATE_TEST_OPERATOR(r, data, op)                                    \
    VERIF_FLOAT(op, data, GET_OUT_TYPE(op, ACCESS_TUPLE_TYPE(data)))

// Iterate over the operations
#define GENERATE_TEST_SIZE(r, type, size)                                      \
  BOOST_PP_LIST_FOR_EACH(GENERATE_TEST_OPERATOR, (type, size), BINARY_OPS)

/* Iterate over the sizes. For some reason you can't have nested
   \c LIST_FOR_EACH so we resort to sequences
*/
#define GENERATE_TEST_TYPE(type)                                               \
  BOOST_PP_SEQ_FOR_EACH(GENERATE_TEST_SIZE, type, SIZES)

#define ALL_TESTS                                                              \
  GENERATE_TEST_TYPE((char,              char,   0));                       \
  GENERATE_TEST_TYPE((unsigned char,     uchar,  0));                       \
  GENERATE_TEST_TYPE((short,             short,  0));                       \
  GENERATE_TEST_TYPE((short int,         ushort, 0));                       \
  GENERATE_TEST_TYPE((int,               int,    0));                       \
  GENERATE_TEST_TYPE((unsigned int,      uint,   0));                       \
  GENERATE_TEST_TYPE((long,              long,   0));                       \
  GENERATE_TEST_TYPE((unsigned long int, ulong,  0));                       \
  GENERATE_TEST_TYPE((float,             float,  1));                       \
  GENERATE_TEST_TYPE((double,            double, 1));

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
#ifndef TRISYCL_OPENCL
          ALL_TESTS
#endif
        }); // End of our commands for this queue
      });
  } // End scope, so we wait for the queue to complete

   return 0;
}
