#ifndef TRISYCL_SYCL_PIPE_RESERVATION_HPP
#define TRISYCL_SYCL_PIPE_RESERVATION_HPP

/** \file The reservation station for OpenCL SYCL pipe accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <iterator>
#include <memory>

#include "triSYCL/pipe_reservation/detail/pipe_reservation.hpp"

namespace trisycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The pipe reservation station allows to reserve an array-like view
    inside the pipe for ordered race-free access from various
    work-items for example
 */
template <typename PipeAccessor>
struct pipe_reservation {
  using accessor_type = PipeAccessor;
  static constexpr bool blocking =
    (accessor_type::target == trisycl::access::target::blocking_pipe);
  using accessor_detail = typename accessor_type::accessor_detail;
  /// The STL-like types
  using value_type = typename accessor_type::value_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using size_type = std::size_t;
  using difference_type = ptrdiff_t;
  using iterator =
    typename detail::pipe_reservation<accessor_detail>::iterator;
  using const_iterator =
    typename detail::pipe_reservation<accessor_detail>::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /** Point to the underlying implementation that can be shared in the
      SYCL model with a handler semantics */
  typename std::shared_ptr<detail::pipe_reservation<accessor_detail>>
  implementation;

  /** Use default constructors so that we can create a new buffer copy
      from another one, with either a l-value or a r-value (for
      std::move() for example).

      Since we just copy the shared_ptr<> above, this is where/how the
      sharing magic is happening with reference counting in this case.
  */
  pipe_reservation() = default;


  /// Create a pipe_reservation for an accessor and a number of elements
  pipe_reservation(accessor_type &accessor, std::size_t s)
    : implementation {
    new detail::pipe_reservation<accessor_detail> {
      get_pipe_detail(accessor), s }
  } {}


  /** Create a pipe_reservation from the implementation detail

      This is an internal constructor to allow reserve() on the
      implementation to lift a full-fledged object through
      accessor::reserve().

      \todo Make it private and add required friends
   */
  pipe_reservation(detail::pipe_reservation<accessor_detail> &&pr)
    : implementation {
    new detail::pipe_reservation<accessor_detail> { std::move(pr) } }
  {}


  /** Test if the pipe_reservation has been correctly allocated

      \return true if the pipe_reservation can be used and committed
  */
  operator bool() const {
    return *implementation;
  }


  /// Get the number of reserved element(s)
  std::size_t size() const {
    return implementation->size();
  }


  /// Access to a given element of the reservation
  reference operator[](std::size_t index) const {
    return (*implementation)[index];
  }


  /** Force a commit operation

      Normally the commit is implicitly done in the destructor, but
      sometime it is useful to do it earlier.
  */
  void commit() const {
    return implementation->commit();
  }


  /// Get an iterator on the first element of the reservation station
  iterator begin() const {
    return implementation->begin();
  }


  /// Get an iterator past the end of the reservation station
  iterator end() const {
    return implementation->end();
  }


  /// Build a constant iterator on the first element of the reservation station
  const_iterator cbegin() const {
    return implementation->begin();
  }


  /// Build a constant iterator past the end of the reservation station
  const_iterator cend() const {
    return implementation->end();
  }


  /// Get a reverse iterator on the last element of the reservation station
  reverse_iterator rbegin() const {
    return std::make_reverse_iterator(end());
  }


  /** Get a reverse iterator on the first element past the end of the
      reservation station */
  reverse_iterator rend() const {
    return std::make_reverse_iterator(begin());
  }


  /** Get a constant reverse iterator on the last element of the
      reservation station */
  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }


  /** Get a constant reverse iterator on the first element past the
      end of the reservation station */
  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

};

/// @} End the data Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_RESERVATION_HPP
