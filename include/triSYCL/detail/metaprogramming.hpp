#ifndef TRISYCL_SYCL_DETAIL_METAPROGRAMMING_HPP
#define TRISYCL_SYCL_DETAIL_METAPROGRAMMING_HPP

/** \file Some metaprogramming utilities
 */

#include <tuple>
#include <type_traits>
#include <utility>

namespace trisycl::detail {

/// Assign arguments from a tuple
auto& assign_from_tuple(auto& dest, auto&& tuple) {
  auto expanded_assign = [&](auto&&... elems) mutable {
    // Assign from each tuple element expanded
    dest = { std::forward<decltype(elems)>(elems)... };
  };
  std::apply(expanded_assign, std::forward<decltype(tuple)>(tuple));
  // So we can use the result of the assignment expression if needed
  return dest;
}

}

#endif // TRISYCL_SYCL_DETAIL_METAPROGRAMMING_HPP
