#ifndef TRISYCL_SYCL_DETAIL_ARRAY_TUPLE_HELPERS_HPP
#define TRISYCL_SYCL_DETAIL_ARRAY_TUPLE_HELPERS_HPP

/** \file

    Some helpers to do array-tuple conversions

    Used for example to implement trisycl::vec<> class.

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <tuple>
#include <utility>

namespace trisycl::detail {

/** \addtogroup array_tuple_helpers Helpers to do array and tuple conversion

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
  return { { static_cast<typename V::element_type>(std::get<Is>(t))...} };
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
                "The number of elements in initialization should match the dimension of the vector");

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

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_ARRAY_TUPLE_HELPERS_HPP
