/* Accessors for the operator datatype. */
# define OP_SYMBOL(O)      BOOST_PP_TUPLE_ELEM(5, 0, O)
# define OP_NAME(O)        BOOST_PP_TUPLE_ELEM(5, 1, O)
# define OP_IS_FLOATING(O) BOOST_PP_TUPLE_ELEM(5, 2, O)
# define OP_IS_LOGICAL(O)  BOOST_PP_TUPLE_ELEM(5, 3, O)
# define OP_IS_SHIFT(O)    BOOST_PP_TUPLE_ELEM(5, 4, O)


# define BINARY_OPS                      \
  BOOST_PP_TUPLE_TO_LIST(                \
    18,                                  \
    (                                    \
      ( *  , mul           ,1 ,0 ,0),    \
      ( /  , div           ,1 ,0 ,0),    \
      ( %  , mod           ,0 ,0 ,0),    \
      ( +  , add           ,1 ,0 ,0),    \
      ( -  , sub           ,1 ,0 ,0),    \
      ( << , shift_left    ,0 ,0 ,1),    \
      ( >> , shift_right   ,0 ,0 ,1),    \
      ( <  , less          ,1 ,1 ,0),    \
      ( <= , less_equal    ,1 ,1 ,0),    \
      ( >= , greater_equal ,1 ,1 ,0),    \
      ( >  , greater       ,1 ,1 ,0),    \
      ( == , equal         ,1 ,1 ,0),    \
      ( != , not_equal     ,1 ,1 ,0),    \
      ( &  , bitwise_and   ,0 ,0 ,0),    \
      ( |  , bitwise_or    ,0 ,0 ,0),    \
      ( ^  , bitwise_xor   ,0 ,0 ,0),    \
      ( && , logical_and   ,1 ,1 ,0),    \
      ( || , logical_or    ,1 ,1 ,0)     \
      )                                  \
    )

#define TRISYCL_CHECK_BINARY(OPERATOR, TYPE_IN, TYPE_OUT, SIZE, VAL1, VAL2)    \
  {                                                                            \
    std::initializer_list<TYPE_IN> vil1 VAL1;                                  \
    std::initializer_list<TYPE_IN> vil2 VAL2;                                  \
    using v = vec<TYPE_IN, SIZE>;                                              \
    using va_in = std::valarray<TYPE_IN>;                                      \
    using va_out = std::valarray<TYPE_OUT>;                                    \
    v v1 VAL1;                                                                 \
    v v2 VAL2;                                                                 \
    v v3 = v1 OPERATOR v2;                                                     \
    va_in va1 { vil1 };                                                        \
    va_in va2 { vil2 };                                                        \
    va_out va3 = va1 OPERATOR va2;                                             \
    BOOST_CHECK(equal(v1, va1));                                               \
    BOOST_CHECK(equal(v2, va2));                                               \
    BOOST_CHECK(equal(v3, va3));                                               \
  }

#define TRISYCL_CHECK_EQUAL_OP(OPERATOR, TYPE_IN, TYPE_OUT, SIZE, VAL1, VAL2)  \
  {                                                                            \
    std::initializer_list<TYPE_IN> vil1 VAL1;                                  \
    using v = vec<TYPE_IN, SIZE>;                                              \
    using va_in = std::valarray<TYPE_IN>;                                      \
    using va_out = std::valarray<TYPE_OUT>;                                    \
    v v1 VAL1;                                                                 \
    va_in va1 { vil1 };                                                        \
    v v2 OPERATOR##= v1;                                                       \
    va_out va2 OPERATOR##= va1;                                                \
    BOOST_CHECK(equal(v3, va3));                                               \
  }
