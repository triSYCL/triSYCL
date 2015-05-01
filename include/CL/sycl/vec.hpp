#ifndef TRISYCL_SYCL_VEC_HPP
#define TRISYCL_SYCL_VEC_HPP

/** \file

    Implement small OpenCL vector class

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

namespace cl {
namespace sycl {

/** \addtogroup vector Vector types in SYCL

    @{
*/

/** Helper to construct an array from initializer elements provided as a
    tuple

    The trick is to get the std::index_sequence<> that represent 0,
    1,..., dimension-1 as a variadic template pack Is that we can
    iterate on, in this function.
*/
template <typename V, typename Tuple, size_t... Is>
std::array<typename V::element_type, V::dimension>
tuple_to_array_iterate(Tuple t, std::index_sequence<Is...>) {
  /* The effect is like a static for-loop with Is counting from 0 to
     dimension-1 and thus constructing a uniform initialization { }
     construction from each tuple element:
     { std::get<0>(t), std::get<1>(t), ..., std::get<dimension-1>(t) }

     The static cast is here to avoid the warning when there is a loss
     of precision, for example when initializing an int from a float.
  */
  return { static_cast<typename V::element_type>(std::get<Is>(t))... };
}

/** Construct an array from initializer elements provided as a tuple
 */
template <typename V, typename Tuple>
auto tuple_to_array(Tuple t) {
  /* Construct an index_sequence with 0, 1, ..., (size of the tuple-1)
     so that tuple_to_array_iterate can statically iterate on it */
  return tuple_to_array_iterate<V>(t,
                                   std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}


/** Allows optional expansion of a 1-element tuple to a V::dimension
    tuple to replicate scalar values in vector initialization
*/
template <typename V, typename Tuple, bool expansion = false>
struct expand_to_vector {
  static_assert(V::dimension == std::tuple_size<Tuple>::value,
                "The number of element in initialization should match the dimension of the vector");

  // By default, act as a pass-through and do not do any expansion
  static auto expand(Tuple t) { return t; }

};

/** Specialization in the case we ask for expansion */
template <typename V, typename Tuple>
struct expand_to_vector<V, Tuple, true> {
  static_assert(std::tuple_size<Tuple>::value == 1,
                "Since it is a vector initialization from a scalar there should be only one initializer value");


  /** Construct a tuple from a value

      \param value is used to initialize each tuple element

      \param size is the number of elements of the tuple to be generated

      The trick is to get the std::index_sequence<> that represent 0,
      1,..., dimension-1 as a variadic template pack Is that we can
      iterate on, in this function.
  */
  template <typename Value, size_t... Is>
  static auto fill_tuple(Value e, std::index_sequence<Is...>) {
    /* The effect is like a static for-loop with Is counting from 0 to
       dimension-1 and thus replicating the pattern to have
       make_tuple( (0, e), (1, e), ... (n - 1, e) )

       Since the "," operator is just here to throw away the Is value
       (which is needed for the pack expansion...), at the end this is
       equivalent to:
       make_tuple( e, e, ..., e )
    */
    return std::make_tuple(((void)Is, e)...);
  }


  /** We expand the 1-element tuple by replicating into a tuple with the
      size of the vector */
  static auto expand(Tuple t) {
    return fill_tuple(std::get<0>(t),
                      std::make_index_sequence<V::dimension>{});
  }

};


/** Create the array data of V from a tuple of initializer

    If there is only 1 initializer, this is a scalar initialization of a
    vector and the value is expanded to all the vector elements first.
*/
template <typename V, typename Tuple>
auto expand(Tuple t) {
  return tuple_to_array<V>(expand_to_vector<V,
                           decltype(t),
                           /* Only ask the expansion to all vector
                              element if there only a scalar
                              initializer */
                           std::tuple_size<Tuple>::value == 1>{}.expand(t));
}


/** Small OpenCL vector class

    \todo add [] operator

    \todo add iterators on elements, with begin() and end()

    \todo having vec<> sub-classing array<> instead would solve the
    previous issues

    \todo move the implementation elsewhere

    \todo simplify the helpers by removing some template types since there
    are now inside the vec<> class.
*/
template <typename DataType, size_t NumElements>
class vec {

public:

  /// The actual storage of the vector elements
  std::array<DataType, NumElements> data;

  static const size_t dimension = NumElements;
  using element_type = DataType;


  /** Construct a vec from anything from a scalar (to initialize all the
      elements with this value) up to an aggregate of scalar and vector
      types (in this case the total number of elements must match the size
      of the vector)
  */
  template <typename... Types>
  vec(const Types... args)
    : data (expand<vec>(flatten_to_tuple<vec>(args...))) { }


  /// Use classical constructors too
  vec() = default;


private:

  /** Flattening helper that does not change scalar values but flatten a
      vec<T, n> v into a tuple<T, T,..., T>{ v[0], v[1],..., v[n-1] }

      If we have a vector, just forward its array content since an array
      has also a tuple interface :-) (23.3.2.9 Tuple interface to class
      template array [array.tuple])
  */
  template <typename V, typename Element, size_t s>
  static auto flatten(const vec<Element, s> i) {
    static_assert(s <= V::dimension,
                  "The element i will not fit in the vector");
    return i.data;
  }


  /** If we do not have a vector, just forward it as a tuple up to the
      final initialization.

      \return typically tuple<double>{ 2.4 } from 2.4 input for example
  */
  template <typename V, typename Type>
  static auto flatten(const Type i) {
    return std::forward_as_tuple(i);
  }


 /** Take some initializer values and apply flattening on each value

      \return a tuple of scalar initializer values
   */
  template <typename V, typename... Types>
  static auto flatten_to_tuple(const Types... i) {
    // Concatenate the tuples returned by each flattening
    return std::tuple_cat(flatten<V>(i)...);
  }


  /// \todo To implement
#if 0
  vec<dataT,
      numElements>
  operator+(const vec<dataT, numElements> &rhs) const;
  vec<dataT, numElements>
  operator-(const vec<dataT, numElements> &rhs) const;
  vec<dataT, numElements>
  operator*(const vec<dataT, numElements> &rhs) const;
  vec<dataT, numElements>
  operator/(const vec<dataT, numElements> &rhs) const;
  vec<dataT, numElements>
  operator+=(const vec<dataT, numElements> &rhs);
  vec<dataT, numElements>
  operator-=(const vec<dataT, numElements> &rhs);
  vec<dataT, numElements>
  operator*=(const vec<dataT, numElements> &rhs);
  vec<dataT, numElements>
  operator/=(const vec<dataT, numElements> &rhs);
  vec<dataT, numElements>
  operator+(const dataT &rhs) const;
  vec<dataT, numElements>
  operator-(const dataT &rhs) const;
  vec<dataT, numElements>
  operator*(const dataT &rhs) const;
  vec<dataT, numElements>
  operator/(const dataT &rhs) const;
  vec<dataT, numElements>
  operator+=(const dataT &rhs);
  vec<dataT, numElements>
  operator-=(const dataT &rhs);
  vec<dataT, numElements>
  operator*=(const dataT &rhs);
  vec<dataT, numElements>
  operator/=(const dataT &rhs);
  vec<dataT, numElements> &operator=(const vec<dataT, numElements> &rhs);
  vec<dataT, numElements> &operator=(const dataT &rhs);
  bool operator==(const vec<dataT, numElements> &rhs) const;
  bool operator!=(const vec<dataT, numElements> &rhs) const;
  // Swizzle methods (see notes)
  swizzled_vec<T, out_dims> swizzle<int s1, ...>();
#ifdef SYCL_SIMPLE_SWIZZLES
  swizzled_vec<T, 4> xyzw();
  ...
#endif // #ifdef SYCL_SIMPLE_SWIZZLES
#endif
};

  /** A macro to define type alias, such as for type=uchar, size=4 and
      real_type=unsigned char, uchar4 is equivalent to vec<float, 4>
  */
#define TRISYCL_DEFINE_VEC_TYPE_SIZE(type, size, actual_type) \
  using type##size = vec<actual_type, size>;

  /// Declare the vector types of a type for all the sizes
#define TRISYCL_DEFINE_VEC_TYPE(type, actual_type)           \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 1, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 2, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 3, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 4, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 8, actual_type)         \
  TRISYCL_DEFINE_VEC_TYPE_SIZE(type, 16, actual_type)

  /// Declare all the possible vector type aliases
  TRISYCL_DEFINE_VEC_TYPE(char, char)
  TRISYCL_DEFINE_VEC_TYPE(uchar, unsigned char)
  TRISYCL_DEFINE_VEC_TYPE(short, short int)
  TRISYCL_DEFINE_VEC_TYPE(ushort, unsigned short int)
  TRISYCL_DEFINE_VEC_TYPE(int, int)
  TRISYCL_DEFINE_VEC_TYPE(uint, unsigned int)
  TRISYCL_DEFINE_VEC_TYPE(long, long int)
  TRISYCL_DEFINE_VEC_TYPE(ulong, unsigned long int)
  TRISYCL_DEFINE_VEC_TYPE(float, float)
  TRISYCL_DEFINE_VEC_TYPE(double, double)

/// @} End the vector Doxygen group


}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_VEC_HPP
