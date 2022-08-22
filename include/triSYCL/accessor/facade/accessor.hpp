#ifndef TRISYCL_SYCL_ACCESSOR_FACADE_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_FACADE_ACCESSOR_HPP

/** \file A SYCL accessor façade to expose to the end user the
    expected accessor interface on top of low-level SYCL accessor
    concepts

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iterator>
#include <type_traits>
#include <utility>

#include "triSYCL/accessor/mixin/accessor.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"

namespace trisycl::facade {

/** \addtogroup execution Platforms, contexts, accessors and queues
    @{
*/

/// SYCL accessor facade providing multi-dimensional access features
template <typename AccessorMixIn> class accessor : public AccessorMixIn {

  using mixin = AccessorMixIn;

 public:
  using iterator = typename mixin::pointer;
  using const_iterator = typename mixin::const_pointer;
  using reverse_iterator = typename std::reverse_iterator<iterator>;
  using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

  /** Use the accessor with integers à la [i1][i2][i3] or C++23 [i1, i2,...]

      \return decltype(auto) to return either a reference to the final
      element when the indexing has been fully resolved or a proxy
      object to handle the remaining [] */
  template <std::integral... T> decltype(auto) operator[](T... indices) {
    if constexpr (sizeof...(T) == 1)
      /* The [][][] case uses a proxy object to track all the [index]
         and aggregate them to resolve the indexing */
      return
          typename mixin::template track_index<1> { mixin::access }[indices...];
    else
      // Or just the C++23 [i1, i2,...] case
      return mixin::access[indices...];
  }

  /** Use the accessor with integers à la [i1][i2][i3] or C++23 [i1, i2,...]

      \return decltype(auto) to return either a reference to the final
      element when the indexing has been fully resolved or a proxy
      object to handle the remaining [] */
  template <std::integral... T> decltype(auto) operator[](T... indices) const {
    if constexpr (sizeof...(T) == 1)
      /* The [][][] case uses a proxy object to track all the [index]
         and aggregate them to resolve the indexing */
      return
          typename mixin::template track_index<1> { mixin::access }[indices...];
    else
      // Or just the C++23 [i1, i2,...] case
      return mixin::access[indices...];
  }

  /// To use the accessor with [id<>]
  decltype(auto) operator[](const id<mixin::rank()>& index) {
    return mixin::tuple_indexed_access(index);
  }

  /// To use the accessor with [id<>]
  decltype(auto) operator[](const id<mixin::rank()>& index) const {
    return mixin::tuple_indexed_access(index);
  }

  /// To use an accessor with [item<>]
  decltype(auto) operator[](const item<mixin::rank()>& index) {
    return (*this)[index.get()];
  }

  /// To use an accessor with [item<>]
  decltype(auto) operator[](const item<mixin::rank()>& index) const {
    return (*this)[index.get()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add to the specification because used by HPC-GPU slide 22
  */
  decltype(auto) operator[](const nd_item<mixin::rank()>& index) {
    return (*this)[index.get_global()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add to the specification because used by HPC-GPU slide 22
  */
  decltype(auto) operator[](const nd_item<mixin::rank()>& index) const {
    return (*this)[index.get_global()];
  }

  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add to the specification
  */
  typename mixin::reference operator*() { return *mixin::data(); }

  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add to the specification?

      \todo Add the concept of 0-dim buffer and accessor for scalar
      and use an implicit conversion to value_type reference to access
      the value with the accessor?
  */
  typename mixin::reference operator*() const { return *mixin::data(); }

  /** Return the pointer to the data

      \todo Implement the various pointer address spaces
  */
  auto get_pointer() { return mixin::data(); }

  /** Forward all the iterator functions to the implementation

      \todo Add these functions to the specification

      \todo The fact that the lambda capture make a const copy of the
      accessor is not yet elegantly managed... The issue is that
      begin()/end() dispatch is made according to the accessor
      constness and not from the array member constness...

      \todo try to solve it by using some enable_if on array
      constness?

      \todo The issue is that the end may not be known if it is
      implemented by a raw OpenCL cl_mem... So only provide on the
      device the iterators related to the start? Actually the accessor
      needs to know a part of the shape to have the multidimentional
      addressing. So this only require a size_t more...

      \todo Factor out these in a template helper

      \todo Do we need this in detail::accessor too or only in accessor?
  */

  iterator begin() { return mixin::data(); }

  iterator end() { return mixin::data() + mixin::get_count(); }

  const_iterator cbegin() { return mixin::data(); }

  const_iterator cend() { return mixin::data() + mixin::get_count(); }

  reverse_iterator rbegin() { return std::reverse_iterator(end()); }

  reverse_iterator rend() {
    return std::reverse_iterator(begin());
    ;
  }

  const_reverse_iterator crbegin() const {
    return std::reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return std::reverse_iterator(cbegin());
  }
};

/// @} to end the Doxygen group

} // namespace trisycl::facade

#endif // TRISYCL_SYCL_ACCESSOR_FACADE_ACCESSOR_HPP
