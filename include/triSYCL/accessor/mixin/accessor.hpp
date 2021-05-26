#ifndef TRISYCL_SYCL_ACCESSOR_MIXIN_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_MIXIN_ACCESSOR_HPP

/** \file A SYCL accessor mixin to implement more easily a low-level
    SYCL accessor concept on top of some concrete storage

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <experimental/mdspan>
#include <tuple>
#include <type_traits>
#include <utility>

#include "triSYCL/range.hpp"

namespace trisycl::mixin {

/** \addtogroup execution Platforms, contexts, accessors and queues
    @{
*/

/// SYCL accessor mixin providing multi-dimensional access features
template <typename T, int Dimensions> class accessor {

 public:
  /** Extension to SYCL: provide pieces of STL container interface
      from mdspan */
  using element_type = T;
  /** Even if the buffer is read-only use a non-const type so at least
      the current implementation can copy the data too */
  using value_type = std::remove_cv_t<element_type>;

  // Useful to handle buffers initialized from const values
  using non_const_pointer = value_type*;

  /** Get the number of dimensions of the buffer

      Name inspired from ISO C++ P0009 mdspan papers
  */
  static auto constexpr rank() { return Dimensions; }

 protected:
  /// Create an mdspan std::experimental::extents object with a
  /// dynamic size for each extent
  template <std::size_t... I>
  static constexpr auto make_dynamic_extents(std::index_sequence<I...>) {
    return std::experimental::extents
        // This repeats n times the std::experimental::dynamic_extent
        <((void)I, std::experimental::dynamic_extent)...> {};
  };

  /// Create an mdspan std::experimental::extents type with a
  /// dynamic size for each extent
  using dynamic_extents =
      decltype(make_dynamic_extents(std::make_index_sequence<Dimensions> {}));

  /// The memory lay-out of a buffer is a dynamic multidimensional array
  using mdspan = std::experimental::basic_mdspan<element_type, dynamic_extents>;

  /** This is the multi-dimensional interface to the data that may point
      to either allocation in the case of storage managed by SYCL itself
      or to some other memory location in the case of host memory or
      storage<> abstraction use
  */
  mdspan access;

  /** Cast a SYCL range into a mdspan index array, which is an array
      of std::size_t into an array of std::ptrdiff_t
  */
  const std::array<typename mdspan::index_type, rank()>&
  extents_cast(const range<Dimensions>& r) {
    return reinterpret_cast<
        const std::array<typename mdspan::index_type, rank()>&>(r);
  }

 public:
  /// Pointer type to the elements
  using pointer = typename mdspan::pointer;

  /// Reference type to the elements
  using reference = typename mdspan::reference;

  /// Create an accessor of dimensions r on top of data storage
  accessor(pointer data, const range<rank()>& r)
      : access { data, extents_cast(r) } {}

  /// Create an accessor from another mdspan
  accessor(const mdspan& m)
      : access { m } {}

  /// Update the accessor to target somewhere else
  void update(pointer data, const range<rank()>& r) {
    access = mdspan { data, extents_cast(r) };
  }

  /** Return a range object representing the size of the buffer in
       terms of number of elements in each dimension as passed to the
       constructor

       \todo Cache it since it is const?
   */
  auto get_range() const {
    range<Dimensions> r;
    for (std::size_t i = 0; i < Dimensions; ++i)
      r[i] = access.extent(i);
    return r;
  }

  /** Returns the total number of elements in the buffer

      Equal to get_range()[0] * ... * get_range()[Dimensions-1].

      \todo Move these kinds of functions into a mixin between buffers
      and accessors?

      \todo Cache it since it is const?
  */
  std::size_t get_count() const {
    return access.mapping().required_span_size();
  }

  /** Returns the size of the buffer storage in bytes

      \todo rename to something else. In
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0122r0.pdf
      it is named bytes() for example

      \todo Cache it since it is const?
  */
  std::size_t get_size() const { return get_count() * sizeof(value_type); }

  /// Get the underlying storage
  auto data() { return access.data(); }

  /** Proxy object to transform an expression like
      accessor[i1][i2][i3] into the implementation mdpsan(i1,i2,i3)
      one index at a time.

      It gathers intermediate [index] to finally call the mdspan
      indexing operator once they are all available

      \parameter[in] N is the number of indices which can be stored in
      this proxy
  */
  template <std::size_t N> struct track_index {
    /// Keep a reference to the mdspan to eventually resolve the
    /// indexing
    mdspan& mds;

    /// The list of indices in the order of [i1][i2][i3]...
    std::array<std::size_t, N> indices;

    /// Construct the initial tracking object from the accessor
    track_index(accessor& a)
        : mds { a.access } {}

    /// Create a tracking object from an mdspan and a list of indices
    template <typename... Index>
    track_index(mdspan& m, Index&&... inds)
        : mds { m }
          // Typically there will be N - 1 indices in the array of size N
        , indices { std::forward<Index>(inds)... } {}

    /// The individual indexing operator
    decltype(auto) operator[](std::size_t index) {
      // Keep track of the new index in the last element
      indices[N - 1] = index;
      if constexpr (N == accessor::rank())
        // If we have accumulated all the indices, call the mdspan
        // indexing function
        return std::invoke(mds, indices);
      else
        // Otherwise return a tracker from the current indices and
        // with 1 more room for the next index
        return std::make_from_tuple<track_index<N + 1>>(
            std::tuple_cat(std::forward_as_tuple(mds), indices));
    }
  };
};

/// @} to end the Doxygen group

} // namespace trisycl::mixin

#endif // TRISYCL_SYCL_ACCESSOR_MIXIN_ACCESSOR_HPP
