#ifndef TRISYCL_SYCL_VEC_DETAIL_VEC_HPP
#define TRISYCL_SYCL_VEC_DETAIL_VEC_HPP

/** \file

    Implement the detail base OpenCL vector class

    Ronan at Keryell point FR
    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/detail/array_tuple_helpers.hpp"

namespace cl::sycl::detail {

/** Small OpenCL vector class
 */
template <typename DataType, int NumElements>
class vec : public detail::small_array<DataType,
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

public:

  /// Return the number of elements in the vector
  auto get_count() const {
    return NumElements;
  }

  /// Return the number of bytes
  auto get_size() const {
    return NumElements * sizeof(DataType);
  }

  /// \todo To implement swizzles
};


};

#endif
