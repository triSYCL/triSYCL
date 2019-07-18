#ifndef TRISYCL_SYCL_VEC_DETAIL_VEC_HPP
#define TRISYCL_SYCL_VEC_DETAIL_VEC_HPP

#include <cstring>
#include <utility>

/** \file

    Implement the detail base OpenCL vector class

    Ronan at Keryell point FR
    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/detail/alignment_helper.hpp"
#include "triSYCL/detail/array_tuple_helpers.hpp"

namespace trisycl::detail {

template <typename, int>
class vec;
template <typename DataType, int numElements>
using __swizzled_base_vec__ = detail::vec<DataType, numElements>;

/** Small OpenCL vector class
 */
template <typename DataType, int NumElements>
class alignas(alignment_v<::trisycl::vec<DataType, NumElements>>)
  vec : public detail::small_array<DataType,
                                  ::trisycl::vec<DataType, NumElements>,
                                  NumElements> {
  using basic_type = typename detail::small_array<DataType,
                                                  ::trisycl::vec<DataType,
                                                                 NumElements>,
                                                  NumElements>;
public:

  /** Construct a vec from anything from a scalar (to initialize all the
      elements with this value) up to an aggregate of scalar and vector
      types (in this case the total number of elements must match the size
      of the vector)
  */
  template <typename... Types>
  vec(const Types... args)
    : basic_type { detail::expand<vec>(flatten_to_tuple<vec>(args...)) } { }


/// Use classical constructors too
  vec() = default;


  // Inherit of all the constructors
  using basic_type::basic_type;

private:

  /** Flattening helper that does not change scalar values but flatten a
      vec<T, n> v into a tuple<T, T,..., T>{ v[0], v[1],..., v[n-1] }

      If we have a vector, just forward its array content since an array
      has also a tuple interface :-) (23.3.2.9 Tuple interface to class
      template array [array.tuple])
  */
  template <typename V, typename Element, int s>
  static auto flatten(const vec<Element, s> i) {
    static_assert(s <= V::dimension,
                  "The element i will not fit in the vector");
    return static_cast<std::array<Element, s>>(i);
  }

  template <typename V, typename Element, int s>
  static auto flatten(const ::trisycl::vec<Element, s> i) {
    static_assert(s <= V::dimension,
                  "The element i will not fit in the vector");
    return static_cast<std::array<Element, s>>(i);
  }


  /** If we do not have a vector, just forward it as a tuple up to the
      final initialization.

      \return typically tuple<double>{ 2.4 } from 2.4 input for example
  */
  template <typename V, typename Type>
  static auto flatten(const Type i) {
    return std::make_tuple(i);
  }


 /** Take some initializer values and apply flattening on each value

      \return a tuple of scalar initializer values
   */
  template <typename V, typename... Types>
  static auto flatten_to_tuple(const Types... i) {
    // Concatenate the tuples returned by each flattening
    return std::tuple_cat(flatten<V>(i)...);
  }

  /* apply_unary_functor_impl & apply_unary_functor generate a sequence from
    a passed in vector and then execute a function across the range of the
    vector returning the result as a new vector.
  */
  template <typename T, typename F, std::size_t... Is>
  static auto apply_unary_functor_impl(T tuple, F f,
                                       std::index_sequence<Is...>) {
    return ::trisycl::vec<DataType, NumElements>{f(std::get<Is>(tuple))...};
  }

  template<typename T, typename F>
  static auto apply_unary_functor(const T x, F f) {
    auto tuple = flatten_to_tuple<T>(x);
    return apply_unary_functor_impl(tuple, f,
        std::make_index_sequence<std::tuple_size<decltype(tuple)>::value>());
  }

  /* apply_binary_functor_impl & apply_binary_functor generate two sequences
   from a passed in vector and then execute a function across the range of
    vector x returning the result as a new vector.
  */
  template<typename T, typename T2, typename F, std::size_t... Is>
  static auto apply_binary_functor_impl(T xTuple, T2 yTuple, F f,
    std::index_sequence<Is...>) {
    return ::trisycl::vec<DataType, NumElements>{f(std::get<Is>(xTuple),
       std::get<Is>(yTuple))...};
  }

  template<typename T, typename T2, typename F>
  static auto apply_binary_functor(const T x, const T2 y, F f) {
    auto xTuple = flatten_to_tuple<T>(x);
    auto yTuple = flatten_to_tuple<T>(y);
    return apply_binary_functor_impl(xTuple, yTuple, f,
        std::make_index_sequence<std::tuple_size<decltype(xTuple)>::value>());
  }

protected:

  /// Apply a swizzle operation from swizzleIndexes parameters
  template<typename... Ts>
  __swizzled_base_vec__<DataType, sizeof...(Ts)>
  swizzle(Ts... swizzleIndexes) const {
    // Construct a new vector from an elemental swizzle of each element
    return __swizzled_base_vec__<DataType, sizeof...(Ts)> {
      (*this)[swizzleIndexes]... };
  }

public:

  /// Return the number of elements in the vector
  auto get_count() const {
    return NumElements;
  }

  /// Return the number of bytes
  auto get_size() const {
    // The alignment for the vec classes are equivalent to the vec's size e.g.
    // a vec of 4 floats would be aligned to 16 bytes which is also the vec's
    // size.
    return alignment_v<::trisycl::vec<DataType, NumElements>>;
  }

  template<typename convertT, rounding_mode roundingMode>
  vec<convertT, NumElements> convert() const {
    vec<convertT, NumElements> result;

    assert(result.get_count() == this->get_count());
    for (int n = 0; n < NumElements; n++) {
      result[n] = (*this)[n];
    }
    return result;
  };

  template<typename asT> asT as() const {
    asT result;
    assert(result.get_size() == this->get_size());
    std::memcpy(result.data(), this->data(), this->get_size());
    return result;
  };

  // Swizzle methods (see notes)
  template <int... swizzleIndexs>
  __swizzled_vec__<DataType, sizeof...(swizzleIndexs)> swizzle() const {
    return static_cast<__swizzled_vec__<DataType, sizeof...(swizzleIndexs)>>(
        swizzle(swizzleIndexs...));
  }

  // Applies a function across each element of the vector to generate a new
  // vector, it leaves the original vector unaltered.
  template <typename F>
  auto map(F f) const {
    return apply_unary_functor(*this, f);
  }

  // Applies a binary function across each element of the calling vector and the
  // passed in vector x to generate a new vector. It leaves both the original
  // vectors unaltered.
  //
  // Note: could in theory be made less restrictive and allow zipping with a
  // scalar type for x and allow zipping across two differing sized vectors.
  // It would require an extra apply_binary_functor overload and a test to find
  // the smallest index_sequence between the calling vector and x.
  template <typename T, int size, typename F>
  auto zip(::trisycl::vec<T, size> x, F f) const {
     static_assert(size == NumElements,
       "zip currently does not support vec's of differing element size");
     return apply_binary_functor(*this, x, f);
  }

};


};

#endif
