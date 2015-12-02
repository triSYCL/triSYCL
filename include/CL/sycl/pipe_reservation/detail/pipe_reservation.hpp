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
    public std::enable_shared_from_this<pipe_reservation<PipeAccessor>> {

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

public:

  pipe_reservation(detail::pipe<value_type> &p, std::size_t s) : p { p } {
    ok = p.reserve(s, rid);
  }


  operator bool() {
    return ok;
  }


  iterator begin() {
    return rid->start;
  }


  iterator end() {
    return rid->start + rid->size;;
  }


  std::size_t size() {
    return rid->size;
  }


  reference operator[](std::size_t index) {
    return rid->start[index];
  }


  void commit() {
    if (ok) {
      rid->ready = true;
      p.move_reservation_forward();
      ok = false;
    }
  }


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
