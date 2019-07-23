#ifndef TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP
#define TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP

/** \file This is a small array class to build range<>, id<>, etc.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>

#include <boost/operators.hpp>

#include "triSYCL/detail/debug.hpp"


namespace trisycl::detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/


/** Helper macro to declare a vector operation with the given side-effect
    operator.
    This handles both a[i] op b[i] and a[i] op b, where b is a BasicType.
*/
#define TRISYCL_BOOST_OPERATOR_VECTOR_OP(op)            \
  FinalType operator op(const FinalType &rhs) {         \
    for (std::size_t i = 0; i != Dims; ++i)             \
      (*this)[i] op rhs[i];                             \
    return *this;                                       \
  }                                                     \
  FinalType operator op(const BasicType &rhs) {         \
    for (std::size_t i = 0; i != Dims; ++i)             \
      (*this)[i] op rhs;                                \
    return *this;                                       \
  }                                                     \

/** Helper macro to declare a vector operation returning a new
    type containing the result of the operator.

    This handles both a[] op b[] and a[] op b, where b is a BasicType.
*/
#define TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(op)          \
  FinalType operator op(const FinalType &rhs) {         \
    FinalType res;                                      \
    for (std::size_t i = 0; i != Dims; ++i)             \
      res[i] = (*this)[i] op rhs[i];                    \
    return res;                                         \
  }                                                     \
  FinalType operator op(const BasicType &rhs) {         \
    FinalType res;                                      \
    for (std::size_t i = 0; i != Dims; ++i)             \
      res[i] = (*this)[i] op rhs;                       \
    return res;                                         \
  }


/** Define a multi-dimensional index, used for example to locate a work
    item or a buffer element

    Unfortunately, even if std::array is an aggregate class allowing
    native list initialization, it is no longer an aggregate if we derive
    from an aggregate. Thus we have to redeclare the constructors.

    \param BasicType is the type element, such as int

    \param Dims is the dimension number, typically between 1 and 3

    \param FinalType is the final type, such as range<> or id<>, so that
    boost::operator can return the right type

    \param EnableArgsConstructor adds a constructors from Dims variadic
    elements when true. It is false by default.

    std::array<> provides the collection concept, with .size(), == and !=
    too.

    Can't use boost::shiftable, as it doesn't define the U, T overload
    outside the class, but when that is added, it gets ambiguity problems.
*/
template <typename BasicType,
          typename FinalType,
          std::size_t Dims,
          bool EnableArgsConstructor = false>
struct small_array : std::array<BasicType, Dims>,
  // To have all the usual arithmetic operations on this type and bitwise
  // operations. Note these opeation classes are chained via:
  // https://www.boost.org/doc/libs/1_69_0/libs/utility/operators.htm#chaining
  // without the chaining the empty base class optimization will break and the
  // class will bloat other types like vec.hpp
  boost::euclidean_ring_operators<FinalType,
  boost::euclidean_ring_operators<FinalType, BasicType,
  boost::bitwise<FinalType,
  boost::bitwise<FinalType, BasicType>>>>,
  // Already provided by array<> lexicographically:
  // boost::equality_comparable<FinalType>,
  // boost::less_than_comparable<FinalType>,
  // Add a display() method
  detail::display_vector<FinalType> {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = Dims;

  /* Note that constexpr size() from the underlying std::array provides
     the same functionality */
  static const size_t dimension = Dims;
  using element_type = BasicType;


  /** A constructor from another array

      Make it explicit to avoid spurious range<> constructions from int *
      for example
  */
  template <typename SourceType>
  small_array(const SourceType src[Dims]) {
    // (*this)[0] is the first element of the underlying array
    std::copy_n(src, Dims, &(*this)[0]);
  }


  /// A constructor from another small_array of the same size
  template <typename SourceBasicType,
            typename SourceFinalType,
            bool SourceEnableArgsConstructor>
  small_array(const small_array<SourceBasicType,
              SourceFinalType,
              Dims,
              SourceEnableArgsConstructor> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
  }


  /** Initialize the array from a list of elements

      Strangely, even when using the array constructors, the
      initialization of the aggregate is not available. So recreate an
      equivalent here.

      Since there are inherited types that defines some constructors with
      some conflicts, make it optional here, according to
      EnableArgsConstructor template parameter.
   */
  template <typename... Types,
            // Just to make enable_if depend of the template and work
            bool Depend = true,
            typename = typename std::enable_if_t<EnableArgsConstructor
                                                 && Depend>>
  small_array(const Types &... args)
    : std::array<BasicType, Dims> {
    // Allow a loss of precision in initialization with the static_cast
    { static_cast<BasicType>(args)... }
  }
  {
    static_assert(sizeof...(args) == Dims,
                  "The number of initializing elements should match "
                  "the dimension");
  }


  /// Construct a small_array from a std::array
  template <typename SourceBasicType>
  small_array(const std::array<SourceBasicType, Dims> &src)
  : std::array<BasicType, Dims>(src) {}


  /// Keep other constructors from the underlying std::array
  using std::array<BasicType, Dims>::array;

  /// Keep the synthesized constructors
  small_array() = default;

  /// Return the element of the array
  auto get(std::size_t index) const {
    return (*this)[index];
  }

  /* Implement minimal methods boost::euclidean_ring_operators needs to
     generate everything */
  /// Add + like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(+=)

  /// Add - like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(-=)

  /// Add * like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(*=)

  /// Add / like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(/=)

  /// Add % like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(%=)

  /// Add << like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(<<=)

  /// Add >> like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(>>=)

  /// Add & like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(&=)

  /// Add ^ like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(^=)

  /// Add | like operations on the id<> and others
  TRISYCL_BOOST_OPERATOR_VECTOR_OP(|=)

  /// Add && operations on the id<> and others
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(&&)

  /// Add || operations on the id<> and others
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(||)

  /// Add comparison operations on the id<> and others
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(<)
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(>)
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(<=)
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(>=)

  /// Add shiftable operators on the vector op
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(<<)
  TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(>>)

  /** Since the boost::operator work on the small_array, add an implicit
      conversion to produce the expected type */
  operator FinalType () {
    return *static_cast<FinalType *>(this);
  }

};

/** Helper macro to declare out-of-class operators for b vs a[]
    operations.
*/
#define TRISYCL_OPERATOR_BASIC_TYPE_OP(op)                            \
template <typename BasicType, typename FinalType, std::size_t Dims>   \
struct small_array<BasicType, FinalType, Dims> operator op(           \
  const BasicType &lhs,                                               \
  const struct small_array<BasicType, FinalType, Dims> &rhs)          \
{                                                                     \
  FinalType res;                                                      \
  for (std::size_t i = 0; i != Dims; ++i)                             \
    res[i] = lhs op rhs[i];                                           \
  return res;                                                         \
}

/* Comparison operators without Boost as Boost only specs bool return types,
   and the SYCL spec wants an arrayed of values returned. */
TRISYCL_OPERATOR_BASIC_TYPE_OP(>)
TRISYCL_OPERATOR_BASIC_TYPE_OP(<)
TRISYCL_OPERATOR_BASIC_TYPE_OP(<=)
TRISYCL_OPERATOR_BASIC_TYPE_OP(>=)
/* Shiftable operators without Boost as Boost doesn't specify
   the out-of-class U, T constructors */
TRISYCL_OPERATOR_BASIC_TYPE_OP(<<)
TRISYCL_OPERATOR_BASIC_TYPE_OP(>>)

#undef TRISYCL_OPERATOR_BASIC_TYPE_OP

/** A small array of 1, 2 or 3 elements with the implicit constructors */
template <typename BasicType, typename FinalType, std::size_t Dims>
struct small_array_123 : small_array<BasicType, FinalType, Dims> {
  static_assert(1 <= Dims && Dims <= 3,
                "Dimensions are between 1 and 3");
};


/** Use some specializations so that some function overloads can be
    determined according to some implicit constructors and to have an
    implicit conversion from/to BasicType (such as an int typically) if
    Dimensions = 1
*/
template <typename BasicType, typename FinalType>
struct small_array_123<BasicType, FinalType, 1>
  : public small_array<BasicType, FinalType, 1> {
  /// A 1-D constructor to have implicit conversion from from 1 integer
  /// and automatic inference of the dimensionality
  small_array_123(BasicType x) {
    (*this)[0] = x;
  }


  /// Keep other constructors
  small_array_123() = default;

  using small_array<BasicType, FinalType, 1>::small_array;
};


template <typename BasicType, typename FinalType>
struct small_array_123<BasicType, FinalType, 2>
  : public small_array<BasicType, FinalType, 2> {
  /// A 2-D constructor to have implicit conversion from from 2 integers
  /// and automatic inference of the dimensionality
  small_array_123(BasicType x, BasicType y) {
    (*this)[0] = x;
    (*this)[1] = y;
  }


  /** Broadcasting constructor initializing all the elements with the
      same value

      \todo Add to the specification of the range, id...
  */
  explicit small_array_123(BasicType e) : small_array_123 { e, e } { }


  /// Keep other constructors
  small_array_123() = default;

  using small_array<BasicType, FinalType, 2>::small_array;
};


template <typename BasicType, typename FinalType>
struct small_array_123<BasicType, FinalType, 3>
  : public small_array<BasicType, FinalType, 3> {
  /// A 3-D constructor to have implicit conversion from from 3 integers
  /// and automatic inference of the dimensionality
  small_array_123(BasicType x, BasicType y, BasicType z) {
    (*this)[0] = x;
    (*this)[1] = y;
    (*this)[2] = z;
  }


  /** Broadcasting constructor initializing all the elements with the
      same value

      \todo Add to the specification of the range, id...
  */
  explicit small_array_123(BasicType e) : small_array_123 { e, e, e } { }


  /// Keep other constructors
  small_array_123() = default;

  using small_array<BasicType, FinalType, 3>::small_array;
};

/// @} End the helpers Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP
