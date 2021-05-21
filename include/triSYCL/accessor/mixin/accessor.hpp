#ifndef TRISYCL_SYCL_ACCESSOR_MIXIN_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_MIXIN_ACCESSOR_HPP

/** \file A SYCL accessor mixin to implement more easily a low-level
    SYCL accessor concept on top of some concrete storage

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <experimental/mdspan>
#include <type_traits>
#include <utility>

#include "triSYCL/range.hpp"

namespace trisycl::mixin {

/** \addtogroup execution Platforms, contexts, accessors and queues
    @{
*/

/// SYCL accessor mixin providing multi-dimensional access features
template <typename T, int Dimensions> class accessor {

 protected:
  /** Extension to SYCL: provide pieces of STL container interface
      from mdspan */
  using element_type = T;
  /** Even if the buffer is read-only use a non-const type so at least
      the current implementation can copy the data too */
  using value_type = std::remove_cv_t<element_type>;

  /** Get the number of dimensions of the buffer

      Name inspired from ISO C++ P0009 mdspan papers
  */
  static auto constexpr rank() { return Dimensions; }

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

  /// Pointer type to the elements
  using pointer = typename mdspan::pointer;

  /// Reference type to the elements
  using reference = typename mdspan::reference;

  /// Create an accessor of dimensions r on top of data storage
  accessor(pointer data, const range<rank()>& r)
      : access { data, extents_cast(r) } {}

  /// Update the accessor to target somewhere else
  void update(pointer data, const range<rank()>& r) {
    access = mdspan { data, extents_cast(r) };
  }
};

/// @} to end the Doxygen group

} // namespace trisycl::mixin

#endif // TRISYCL_SYCL_ACCESSOR_MIXIN_ACCESSOR_HPP
