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

#include "CL/sycl/detail/alignment_helper.hpp"
#include "CL/sycl/detail/array_tuple_helpers.hpp"

namespace cl::sycl::detail {

template <typename, int>
class vec;
template <typename DataType, int numElements>
using __swizzled_base_vec__ = detail::vec<DataType, numElements>;

/** Small OpenCL vector class
 */
template <typename DataType, int NumElements>
class alignas(alignment_v<cl::sycl::vec<DataType, NumElements>>)
  vec : public detail::small_array<DataType,
                                  cl::sycl::vec<DataType, NumElements>,
                                  NumElements> {
  using basic_type = typename detail::small_array<DataType,
                                                  cl::sycl::vec<DataType, NumElements>,
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
  static auto flatten(const cl::sycl::vec<Element, s> i) {
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

  // This is a little more generic than a zip or map as it relies on the
  // captures of the lambda to do anything interesting rather than passing
  // vectors to it. All the apply_functor/apply_functor_impl do is work out
  // an index sequence for the vec and invoked it then generate a new
  // cl::sycl::vec from the result.
  template<typename F, std::size_t... Is>
  static auto apply_functor_impl(F f, std::index_sequence<Is...>) {
    return cl::sycl::vec<DataType, NumElements>{f(Is)...};
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
    return alignment_v<cl::sycl::vec<DataType, NumElements>>;
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

  // Applies a "functor" across an index the size of the cl::sycl::vec type it's
  // invoked by: e.g. cl::sycl::vec<float, 4>::apply_functor(lambda);
  //
  // It creates a new cl::sycl::vec of the type its invoked by and returns it,
  // it does not alter the state of values captured by the lambda directly. It's
  // most likely possible to alter the captured values if you capture by
  // reference and manipulate them inside the passed in lambda however.
  //
  // \todo Perhaps possible to get rid of the pointless vec_t{0} creation if
  // flatten_to_tuple is made more compile time friendly and has a variation
  // that requires only template parameters.
  template<typename F>
  static auto apply_functor(F f) {
    using vec_t = cl::sycl::vec<DataType, NumElements>;
    return apply_functor_impl(f, std::make_index_sequence<
        std::tuple_size<decltype(flatten_to_tuple<vec_t>(vec_t{0}))>::value>());
  }

};


};

#endif
