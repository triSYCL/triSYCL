#ifndef TRISYCL_SYCL_PIPE_RESERVATION_DETAIL_PIPE_RESERVATION_HPP
#define TRISYCL_SYCL_PIPE_RESERVATION_DETAIL_PIPE_RESERVATION_HPP

/** \file The OpenCL SYCL pipe accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "CL/sycl/pipe/detail/pipe.hpp"

namespace cl {
namespace sycl {
namespace detail {

template <typename T,
          std::size_t Dimensions,
          access::mode Mode,
          access::target Target /* = access::global_buffer */>
struct accessor;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

template <typename PipeAccessor>
class pipe_reservation :
    public detail::debug<detail::pipe_reservation<PipeAccessor>> {

  using accessor_type =  PipeAccessor;
  using value_type = typename accessor_type::value_type;
  using reference = typename accessor_type::reference;
  using iterator = typename
    std::deque<detail::reserve_id<value_type>>::iterator;

  /// True if the reservation was successful and still uncommitted
  bool ok;

  /// Point into the reservation buffer. Only valid if ok is true
  iterator rid;

  detail::pipe<value_type> &p;


  /** Test that the reservation is in a usable state

      \todo Throw exception instead
  */
  void assume_validity() {
    assert(ok);
  }

public:

  pipe_reservation(detail::pipe<value_type> &p, std::size_t s) : p { p } {
    ok = p.reserve(s, rid);
  }


  /** No copy constructor with some spurious commit in the destructor
      of the original object
  */
  pipe_reservation(const pipe_reservation &) = delete;


  /// Only a move constructor is required to move it in the shared_ptr
  pipe_reservation(pipe_reservation &&orig) :
    ok {orig.ok },
    rid {orig.rid },
    p { orig.p } {
      /* Even when an object is moved, the destructor of the old
         object is eventually called, so leave the old object in a
         destructable state but without any commit capability */
      orig.ok = false;
    }


  /** Keep the default constructors too

      Otherwise there is no move semantics and the copy is made by
      creating a new reservation and destructing the old one with a
      spurious commit in the meantime....
  */
  pipe_reservation() = default;



  operator bool() {
    return ok;
  }


  iterator begin() {
    assume_validity();
    return rid->start;
  }


  iterator end() {
    assume_validity();
    return rid->start + rid->size;;
  }


  std::size_t size() {
    assume_validity();
    return rid->size;
  }


  reference operator[](std::size_t index) {
    assume_validity();
    TRISYCL_DUMP_T("[] index = " << index
                   << " Reservation write address = " << &(rid->start[index]));

    return rid->start[index];
  }


  /** Commit the reservation station

      \todo Add to the specification that for simplicity a reservation
      can be commited several times but only the first one is taken
      into account
  */
  void commit() {
    if (ok) {
      // If the reservation is in a committable state, commit
      TRISYCL_DUMP_T("Commit");
      rid->ready = true;
      p.move_reservation_forward();
      ok = false;
    }
  }


  /// An implicit commit is made in the destructor
  ~pipe_reservation() {
    commit();
  }

};

/// @} End the data Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_RESERVATION_DETAIL_PIPE_RESERVATION_HPP
