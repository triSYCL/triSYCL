#ifndef TRISYCL_SYCL_DETAIL_SHARED_PTR_IMPLEMENTATION_HPP
#define TRISYCL_SYCL_DETAIL_SHARED_PTR_IMPLEMENTATION_HPP

/** \file Mix-in to add an implementation as shared_ptr with total
    ordering and hashing so that the class can be used with algorithms
    and in (un)ordered containers

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <functional>
#include <memory>

#include <boost/operators.hpp>

namespace trisycl::detail {

/** Provide an implementation as shared_ptr with total ordering and
    hashing to be used with algorithms and in (un)ordered containers

    To be used, a Parent class wanting an Implementation needs to
    inherit from.

    The implementation ends up in a member really named
    "implementation".

    \code
    public detail::shared_ptr_implementation<Parent, Implementation>
    \endcode

    and also inject in std namespace a specialization for
    \code hash<Parent> \endcode
*/
template <typename Parent, typename Implementation>
struct shared_ptr_implementation : public boost::totally_ordered<Parent> {

  /// The implementation forward everything to this... implementation
  std::shared_ptr<Implementation> implementation;

  /// The implementation directly as a shared pointer
  shared_ptr_implementation(std::shared_ptr<Implementation> i)
    : implementation { i } {}


  /// The implementation takes the ownership from a raw pointer
  shared_ptr_implementation(Implementation *i) : implementation { i } {}


  /// Keep all other constructors to have usual shared_ptr behaviour
  shared_ptr_implementation() = default;


  /** Equality operator

      This is generalized by boost::equality_comparable from
      boost::totally_ordered to implement the equality comparable
      concept
  */
  bool operator ==(const Parent &other) const {
    return implementation == other.implementation;
  }


  /** Inferior operator

      This is generalized by boost::less_than_comparable from
      boost::totally_ordered to implement the equality comparable
      concept

      \todo Add this to the spec
  */
  bool operator <(const Parent &other) const {
    return implementation < other.implementation;
  }


  /// Forward the hashing for unordered containers to the implementation
  auto hash() const {
    return std::hash<decltype(implementation)>{}(implementation);
  }

};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_SHARED_PTR_IMPLEMENTATION_HPP
