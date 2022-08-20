#ifndef TRISYCL_SYCL_ID_HPP
#define TRISYCL_SYCL_ID_HPP

/** \file The OpenCL SYCL id<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>

#include "triSYCL/detail/small_array.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

template <int Dimensions, bool with_offset> class item;

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** Define a multi-dimensional index, used for example to locate a work
    item
*/
template <int Dimensions = 1>
class id : public detail::small_array_sycl<
             std::size_t,
             id<Dimensions>,
             Dimensions > {

public:
  /// The number of dimensions of the id
  static auto constexpr rank() { return Dimensions; }

  // Inherit from all the constructors
  using detail::small_array_sycl<std::size_t,
                                id<Dimensions>,
                                Dimensions>::small_array_sycl;

  /// Construct an id from a range
  id(const range<Dimensions>& range_size)
      /** Use the fact we have a constructor of a small_array from a another
          kind of small_array
      */
      : detail::small_array_sycl<std::size_t, id<Dimensions>, Dimensions> {
        range_size
      } {}

  /// Construct an id from an item global_id
  id(const item<Dimensions, true> &rhs)
    : detail::small_array_sycl<std::size_t, id<Dimensions>, Dimensions>
      { rhs.get_id() }
  {}

  /// Default constructor must 0 all elements
  id() : detail::small_array_sycl<std::size_t, id<Dimensions>, Dimensions> { 0 }
  {}
};

/**  User-defined deduction guides to deduce the rank of an id from the number
     of arguments

     For example id { 4, 5, 7 } will define an id<3> { 4, 5, 7 }
*/
template <typename... BasicType> id(BasicType... Args) -> id<sizeof...(Args)>;

/// @} End the parallelism Doxygen group

}

namespace std {
// Declare a tuple-like interface for the sycl::id

/// Export the id dimension as its tuple size
template <int Dimensions> struct tuple_size<trisycl::id<Dimensions>>
    : public std::integral_constant<std::size_t, Dimensions> {};

/// The element of the tuple is the matching id element
template <std::size_t I, int Dimensions>
decltype(auto) get(trisycl::id<Dimensions>&& id) {
  return id.get(I);
}

/// Each tuple element type is the same, the one of any id element
template <std::size_t I, int Dimensions>
struct tuple_element<I, trisycl::id<Dimensions>> {
  using type = typename trisycl::id<Dimensions>::element_type;
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ID_HPP
