#ifndef TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP
#define TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP

/** \file This is a small array class to build range<>, id<>, etc.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <boost/operators.hpp>

#include <range/v3/all.hpp>

#include "triSYCL/detail/global_config.hpp"
#include "triSYCL/detail/array_tuple_helpers.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/metaprogramming.hpp"


namespace trisycl::detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/

/// Tag to define simply inheritance from small_array
struct small_array_tag {};

/// Concept describing a class inheriting from small_array
template <typename T>
concept is_small_array = std::derived_from<T, trisycl::detail::small_array_tag>;

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
#define TRISYCL_LOGICAL_OPERATOR_VECTOR_OP(op)                    \
  FinalType operator op(const FinalType &rhs) const {             \
    FinalType res;                                                \
    for (std::size_t i = 0; i != Dims; ++i)                       \
      res[i] = (*this)[i] op rhs[i];                              \
    return res;                                                   \
  }                                                               \
  /* Skip this for Dims = 1 to avoid ambiguity with implicit type \
     conversion between the vector type and its basic type */     \
  template <typename FT = FinalType,                              \
            typename = std::enable_if_t<Dims != 1, FT>>           \
  FinalType operator op(const BasicType &rhs) {                   \
    FinalType res;                                                \
    for (std::size_t i = 0; i != Dims; ++i)                       \
      res[i] = (*this)[i] op rhs;                                 \
    return res;                                                   \
  }

/** Helper macro to declare a vector unary operation.

    This handles op a.
*/
#define TRISYCL_UNARY_OPERATOR_VECTOR_OP(op)    \
  FinalType operator op() const {               \
    FinalType result;                           \
    for (std::size_t i = 0; i != Dims; ++i)     \
      result[i] = op(*this)[i];                 \
    return result;                              \
  }

/** Helper macro to declare a vector prefix unary operation.

    This handles ++a and --a.
*/
#define TRISYCL_UNARY_PREFIX_OPERATOR_VECTOR_OP(op) \
  FinalType operator op() {                         \
    for (std::size_t i = 0; i != Dims; ++i)         \
      op(*this)[i];                                 \
    return *this;                                   \
  }

/** Helper macro to declare a vector prefix unary operation.

    This handles a++ and a--.
*/
#define TRISYCL_UNARY_POSTFIX_OPERATOR_VECTOR_OP(op)  \
  FinalType operator op(int) {                        \
    FinalType result = *this;                         \
    for (std::size_t i = 0; i != Dims; ++i)           \
      op(*this)[i];                                   \
    return result;                                    \
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

    std::array<> provides the collection concept, with .size(), == and !=
    too.

    Can't use boost::shiftable, as it doesn't define the U, T overload
    outside the class, but when that is added, it gets ambiguity problems.
*/
template <typename BasicType,
          typename FinalType,
          std::size_t Dims>
struct small_array : std::array<BasicType, Dims>,
  // To have all the usual arithmetic operations on this type and bitwise
  // operations. Note these operation classes are chained via:
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
  detail::display_vector<FinalType>,
  // Just to find easily this is a small_array without templates
  detail::small_array_tag {
  /* Note that constexpr size() from the underlying std::array provides
     the same functionality */
  static constexpr size_t dimension = Dims;

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
            typename SourceFinalType>
  small_array(const small_array<SourceBasicType,
              SourceFinalType,
              Dims> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
  }

  /** Initialize the array from a list of exactly Dims elements

      Strangely, even when using the array constructors, the
      initialization of the aggregate is not available. So recreate an
      equivalent here.
  */
  template <typename... Types>
    requires(sizeof...(Types) == Dims)
  small_array(const Types&... args)
      : std::array<BasicType, Dims> {
        // Allow a loss of precision in initialization with the static_cast
        { static_cast<BasicType>(args)... }
      } {}

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

#undef TRISYCL_BOOST_OPERATOR_VECTOR_OP

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

#undef TRISYCL_LOGICAL_OPERATOR_VECTOR_OP

  /// Implement unary operations
  TRISYCL_UNARY_OPERATOR_VECTOR_OP(-)
  TRISYCL_UNARY_OPERATOR_VECTOR_OP(+)

#undef TRISYCL_UNARY_OPERATOR_VECTOR_OP

  /// Implement prefix unary operations
  TRISYCL_UNARY_PREFIX_OPERATOR_VECTOR_OP(--)
  TRISYCL_UNARY_PREFIX_OPERATOR_VECTOR_OP(++)

#undef TRISYCL_UNARY_PREFIX_OPERATOR_VECTOR_OP

  /// Implement postfix unary operations
  TRISYCL_UNARY_POSTFIX_OPERATOR_VECTOR_OP(--)
  TRISYCL_UNARY_POSTFIX_OPERATOR_VECTOR_OP(++)

#undef TRISYCL_UNARY_POSTFIX_OPERATOR_VECTOR_OP

  /** Since the boost::operator work on the small_array, add an implicit
      conversion to produce the expected type */
  operator FinalType() {
    return static_cast<FinalType&>(*this);
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

/** A small array to implement SYCL objects like id, range,
    item... with 1, 2 or 3 elements (or even more if
    detail::allow_any_dimension is true), with the implicit
    constructors */
template <typename BasicType, typename FinalType, std::size_t Dims>
struct small_array_sycl : small_array<BasicType, FinalType, Dims> {
  static_assert(1 <= Dims, "SYCL dimensions are greater than or equal to 1");
  static_assert(detail::allow_any_dimension || Dims <= 3,
                "SYCL dimensions are less than or equal to 3");

  /// Keep other constructors
  small_array_sycl() = default;

  /** Construct from a range-like object inheriting from a
      compatible std::array

      This useful in implementation details for example to construct a
      sycl::id from a sycl::range.
  */
  template <typename Range>
  requires requires {
    ranges::copy(std::declval<Range>(), std::declval<BasicType*>());
  }
  small_array_sycl(Range&& r) {
    // Copy to internal value
    ranges::copy(std::forward<Range>(r), this->begin());
  }

  /** Constructor taking either Dims arguments compatible with BasicType
      or just 1 argument compatible with BasicType and broadcast its
      value to all the elements */
  template <typename... Args>
  // Check that each parameter is convertible to the BasicType
  // Full C++20: requires (std::convertible_to<Args, BasicType> && ...)
  // In the meantime:
  requires requires { (static_cast<BasicType>(std::declval<Args>()), ...); }
  small_array_sycl(Args... args) {
    if constexpr (sizeof...(Args) == Dims) {
      // If there is the right number of elements we initialize each
      // element from each parameter, allowing some narrowing
      static_cast<std::array<BasicType, Dims>&>(
          *this) = { static_cast<BasicType>(args)... };
    } else if constexpr (sizeof...(Args) == 1) {
      // If there is only 1 argument, this is a broadcast
      detail::assign_from_tuple(
          static_cast<std::array<BasicType, Dims>&>(*this),
          detail::expand_to_vector<small_array_sycl, std::tuple<BasicType>,
                                   true> {}
              .expand(args...));
    } else
      static_assert(true, "There should be either 1 argument to broadcast the "
                          "value or 1 value per dimension");
  }

  /** Conversion operator to scalar integral when only 1 dimension

      This is used for example to implicitly convert a sycl::id<1> to
      a std::size_t or int
  */
  template <std::integral To>
    requires(Dims == 1)
  operator To() {
    return this->get(0);
  }
};

/// @} End the helpers Doxygen group

}

namespace std {
// Declare a tuple-like interface for the heirs of the small_array

/// Export the dimension as its tuple size
template <typename T>
  requires trisycl::detail::is_small_array<T>
struct tuple_size<T>
    : public std::integral_constant<std::size_t, T::dimension> {};

/// The element of the tuple is the matching element of the small_array heir
template <std::size_t I, typename T>
  requires trisycl::detail::is_small_array<T> decltype(auto)
get(T&& e) {
  return e.get(I);
}

/// Each tuple element type is the same, the one of any small_array
/// heir element
template <std::size_t I, typename T>
  requires trisycl::detail::is_small_array<T>
struct tuple_element<I, T> {
  using type = typename T::element_type;
};

} // namespace std

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP
