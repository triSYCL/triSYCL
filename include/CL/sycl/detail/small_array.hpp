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

#include <boost/operators.hpp>

#include "CL/sycl/detail/debug.hpp"


namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/


/** Helper macro to declare a vector operation with the given side-effect
    operator */
#define TRISYCL_BOOST_OPERATOR_VECTOR_OP(op)            \
  FinalType operator op(const FinalType& rhs) {         \
    for (std::size_t i = 0; i != Dims; ++i)             \
      (*this)[i] op rhs[i];                             \
    return *this;                                       \
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
*/
template <typename BasicType, typename FinalType, std::size_t Dims>
struct small_array : std::array<BasicType, Dims>,
  // To have all the usual arithmetic operations on this type
  boost::euclidean_ring_operators<FinalType>,
  // Bitwise operations
  boost::bitwise<FinalType>,
  // Shift operations
  boost::shiftable<FinalType>,
  // Already provided by array<> lexicographically:
  // boost::equality_comparable<FinalType>,
  // boost::less_than_comparable<FinalType>,
  // Add a display() method
  detail::display_vector<FinalType> {

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static const auto dimensionality = Dims;


  /** Add a constructor from an other array

      Make it explicit to avoid spurious range<> constructions from int *
      for example
  */
  template <typename SourceType>
  explicit small_array(const SourceType src[Dims]) {
    // (*this)[0] is the first element of the underlying array
    std::copy_n(src, Dims, &(*this)[0]);
  }


  /// Add a constructor from an other small_array of the same size
  template <typename SourceBasicType, typename SourceFinalType>
  small_array(const small_array<SourceBasicType, SourceFinalType, Dims> &src) {
    std::copy_n(&src[0], Dims, &(*this)[0]);
  }


  /// Keep other constructors
  using std::array<BasicType, Dims>::array;

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

  /** Since the boost::operator work on the Small array, add an implicit
      conversion to produce the expected type */
  operator FinalType () {
    return *static_cast<FinalType *>(this);
  }

};


/** A small array of 1, 2 or 3 elements with the implicit constructors */
template <typename BasicType, typename FinalType, std::size_t Dims>
struct small_array_123 : small_array<BasicType, FinalType, Dims> {
  static_assert(1 <= Dims && Dims <= 3,
                "Dimensions are between 1 and 3");
};


/** Use some specializations so that some function overloads can be
    determined according to some implicit constructors and to have an
    implicit conversion from/to BasicType (such as an int typically) if
    dims = 1
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

  /** Conversion so that an for example an id<1> can basically be used
      like an integer */
  operator BasicType() const {
    return (*this)[0];
  }
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


  /// Keep other constructors
  small_array_123() = default;

  using small_array<BasicType, FinalType, 3>::small_array;
};

/// @} End the helpers Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SMALL_ARRAY_HPP
