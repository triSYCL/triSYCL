#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_VIEW_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_VIEW_HPP

/** \file The SYCL buffer view façade behind the scene

    Store the geometry of the buffer but not the accessor methods
    which are in buffer_accessor_view instead.

    \todo It is based on Boost MultiArray for now but to move to
    C++2a md_span

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include <boost/multi_array.hpp>

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor/detail/accessor_traits.hpp"

namespace cl::sycl::detail {

template <typename T,
          int Dimensions>
class buffer_view {

public:

  /// The implementation is a multi_array_ref wrapper
  using array_view_type = boost::multi_array_ref<T, Dimensions>;

  /// The way the buffer is really accessed
  array_view_type array;

  //* Make a buffer_accessor_view from implementation detail
  buffer_accessor_view(const array_view_type &access)
    : array { access }
  {}


  //* Make a buffer_accessor_view from storage and sizes
  buffer_view(pointer storage, const range<Dimensions> &r)
    : buffer_accessor_view { array_view_type { storage, r } }
  {}


  /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
    */
    return range<Dimensions> {
      *(const std::size_t (*)[Dimensions])(array.shape())
        };
  }


  /** Returns the total number of elements behind the accessor

      Equal to get_range()[0] * ... * get_range()[Dimensions-1].

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_count() const {
    return array.num_elements();
  }


  /** Returns the size of the underlying buffer storage in bytes

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_size() const {
    return get_count()*sizeof(value_type);
  }
  /** Return the pointer to the data

      \todo Implement the various pointer address spaces
  */
  auto
  get_pointer() const {
    return array.data();
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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_VIEW_HPP
