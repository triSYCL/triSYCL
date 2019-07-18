#ifndef TRISYCL_SYCL_PIPE_RESERVATION_DETAIL_PIPE_RESERVATION_HPP
#define TRISYCL_SYCL_PIPE_RESERVATION_DETAIL_PIPE_RESERVATION_HPP

/** \file The OpenCL SYCL pipe reservation detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "triSYCL/pipe/detail/pipe.hpp"

namespace trisycl::detail {

template <typename T,
          int Dimensions,
          access::mode Mode,
          access::target Target>
class accessor;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/// The implementation of the pipe reservation station
template <typename PipeAccessor>
class pipe_reservation :
    public detail::debug<detail::pipe_reservation<PipeAccessor>> {
  using accessor_type = PipeAccessor;
  static constexpr bool blocking =
    (accessor_type::target == trisycl::access::target::blocking_pipe);
  using value_type = typename accessor_type::value_type;
  using reference = typename accessor_type::reference;

public:

  using iterator =
    typename detail::pipe<value_type>::implementation_t::iterator;
  using const_iterator =
    typename detail::pipe<value_type>::implementation_t::const_iterator;

  // \todo Add to the specification
  static constexpr access::mode mode = accessor_type::mode;
  static constexpr access::target target = accessor_type::target;

  /**  True if the reservation was successful and still uncommitted. B
       default a pipe_reservation is not reserved and cannot be
       committed */
  bool ok = false;

  /// Point into the reservation buffer. Only valid if ok is true
  typename detail::pipe<value_type>::rid_iterator rid;

  /** Keep a reference on the pipe to access to the data and methods

      Note that with inlining and CSE it should not use more register
      when compiler optimization is in use. */
  detail::pipe<value_type> &p;


  /** Test that the reservation is in a usable state

      \todo Throw exception instead
  */
  void assume_validity() {
    assert(ok);
  }

public:

  /// Create a pipe reservation station that reserves the pipe itself
  pipe_reservation(detail::pipe<value_type> &p, std::size_t s) : p { p } {
    static_assert(mode == access::mode::write
                  || mode == access::mode::read,
                  "A pipe can only be accesed in read or write mode,"
                  " exclusively");

    /* Since this test is constexpr and dependent of a template
       parameter, it should be equivalent to a specialization of the
       method but in a clearer way */
    if (mode == access::mode::write)
      ok = p.reserve_write(s, rid, blocking);
    else
      ok = p.reserve_read(s, rid, blocking);
  }


  /** No copy constructor with some spurious commit in the destructor
      of the original object
  */
  pipe_reservation(const pipe_reservation &) = delete;


  /// Only a move constructor is required to move it into the shared_ptr
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
      spurious commit in the meantime...
  */
  pipe_reservation() = default;


  /** Test if the reservation succeeded and thus if the reservation
      can be committed

      Note that it is up to the user to ensure that all the
      reservation elements have been initialized correctly in the case
      of a write for example
  */
  operator bool() {
    return ok;
  }


  /// Start of the reservation area
  iterator begin() {
    assume_validity();
    return rid->start;
  }


  /// Past the end of the reservation area
  iterator end() {
    assume_validity();
    return rid->start + rid->size;
  }


  /// Get the number of elements in the reservation station
  std::size_t size() {
    assume_validity();
    return rid->size;
  }


  /// Access to an element of the reservation
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
      if (mode == access::mode::write)
        p.move_write_reservation_forward();
      else
        p.move_read_reservation_forward();
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

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_RESERVATION_DETAIL_PIPE_RESERVATION_HPP
