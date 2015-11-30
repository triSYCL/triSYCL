#ifndef TRISYCL_SYCL_PIPE_RESERVATION_HPP
#define TRISYCL_SYCL_PIPE_RESERVATION_HPP

/** \file The reservation station for OpenCL SYCL pipe accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "CL/sycl/pipe_reservation/detail/pipe_reservation.hpp"

namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

template <typename PipeAccessor>
struct pipe_reservation {
  /// The STL-like types
  using accessor_type =  PipeAccessor;
  using value_type = typename accessor_type::value_type;
  using reference = value_type&;
  using const_reference = const value_type&;

  /** Point to the underlying implementation that can be shared in the
      SYCL model with a handler semantics */
  typename std::shared_ptr<detail::pipe_reservation<accessor_type>> implementation;

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
    new detail::pipe_reservation<accessor_type> { accessor, s }
  } {}


  /** Create a pipe_reservation from the implementation detail

      This is an internal constructor to allow reserve() on the
      implementation to lift a full-fledged object through
      accessor::reserve().

      \todo Make it private and add required friends
   */

  pipe_reservation(detail::pipe_reservation<accessor_type> &pr)
    : implementation { pr.shared_from_this() }
  {}


  /** Test if the pipe_reservation has been correctly allocated

      \return true if the pipe_reservation can be used and committed
  */
  operator bool() {
    return *implementation;
  }


  /// \todo Add all kind of iterators

  /// Get an iterator on the first element of the reservation station
  auto begin() {
    return implementation->begin();
  }


  /// Get an iterator past the end of the reservation station
  auto end() {
    return implementation->end();
  }


  /// Get the number of reserved element(s)
  std::size_t size() {
    return implementation->size();
  }


  /// Access to a given element of the reservation
  reference operator[](std::size_t index) {
    return (*implementation)[index];
  }


  /** Force a commit operation

      Normally the commit is implicitly done in the destructor, but
      sometime it is useful to do it earlier.
  */
  void commit() {
    return implementation->commit();
  }

};

/// @} End the data Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_RESERVATION_HPP
