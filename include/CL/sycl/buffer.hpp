#ifndef TRISYCL_SYCL_BUFFER_HPP
#define TRISYCL_SYCL_BUFFER_HPP

/** \file The OpenCL SYCL buffer<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor.hpp"
#include "CL/sycl/buffer/detail/buffer.hpp"
#include "CL/sycl/buffer_allocator.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/range.hpp"
#include "CL/sycl/storage.hpp"

namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    \todo We have some read-write buffers and some read-only buffers,
    according to the constructor called. So we could have some static
    checking for correctness with the accessors used, but we do not have a
    way in the specification to have a read-only buffer type for this.

    \todo there is a naming inconsistency in the specification between
    buffer and accessor on T versus datatype
*/
template <typename T,
          std::size_t Dimensions = 1,
          typename Allocator = buffer_allocator<T>>
struct buffer {
  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using allocator_type = Allocator;

  /** Point to the underlying buffer implementation that can be shared in
      the SYCL model */
  std::shared_ptr<detail::buffer<T, Dimensions>> implementation;

  /** Use default constructors so that we can create a new buffer copy
      from another one, with either a l-value or an r-value (for
      std::move() for example).

      Since we just copy the shared_ptr<> above, this is where/how the
      sharing magic is happening with reference counting in this case.
  */
  buffer() = default;


  /** Create a new read-write buffer with storage managed by SYCL

      \param r defines the size
  */
  buffer(const range<Dimensions> &r, Allocator allocator = {})
    : implementation { new detail::buffer<T, Dimensions> { r } } {}


  /** Create a new read-write buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(T * host_data, range<Dimensions> r, Allocator allocator = {})
    : implementation { new detail::buffer<T, Dimensions> { host_data, r } } {}


  /** Create a new read-only buffer with associated host memory

      \param host_data points to the storage and values used by the buffer

      \param r defines the size
  */
  buffer(const T * host_data, range<Dimensions> r, Allocator allocator = {})
    : implementation { new detail::buffer<T, Dimensions> { host_data, r } } {}


  /** Create a new buffer from a storage abstraction provided by the user

      \param store is the storage back-end to use for the buffer

      \param r defines the size

      The storage object has to exist during all the life of the buffer
      object.

      \todo To be implemented
  */
  buffer(storage<T> &store, range<Dimensions> r, Allocator allocator = {}) { assert(0); }


  /** Create a new read-write allocated 1D buffer initialized from the
      given elements

      \param start_iterator points to the first element to copy

      \param end_iterator points to just after the last element to copy

      \todo Add const to the SYCL specification.

      \todo Generalize this for n-D and provide column-major and row-major
      initialization

      \todo Allow read-only buffer construction too

      \todo Allow initialization from ranges and collections à la STL
  */
  template <typename Iterator,
            /* To force some iterator concept checking to avoid GCC 4.9
               diving into this when initializing from ({ int, int })
               which is a range<> and and not an iterator... */
            typename ValueType =
            typename std::iterator_traits<Iterator>::value_type>
  buffer(Iterator start_iterator,
         Iterator end_iterator,
         Allocator allocator = {}) :
    implementation { new detail::buffer<T, Dimensions> { start_iterator,
                                                         end_iterator } }
  {}


  /** Create a new sub-buffer without allocation to have separate accessors
      later

      \param b is the buffer with the real data

      \param base_index specifies the origin of the sub-buffer inside the
      buffer b

      \param sub_range specifies the size of the sub-buffer

      \todo To be implemented

      \todo Update the specification to replace index by id
  */
  buffer(buffer<T, Dimensions, Allocator> b,
         id<Dimensions> base_index,
         range<Dimensions> sub_range,
         Allocator allocator = {}) { assert(0); }


#ifdef TRISYCL_OPENCL
  /** Create a buffer from an existing OpenCL memory object associated to
      a context after waiting for an event signaling the availability of
      the OpenCL data

      \param mem_object is the OpenCL memory object to use

      \param from_queue is the queue associated to the memory object

      \param available_event specifies the event to wait for if non null

      \todo To be implemented

      \todo Improve the specification to allow CLHPP objects too
  */
  buffer(cl_mem mem_object,
         queue from_queue,
         event available_event,
         Allocator allocator = {}) { assert(0); }
#endif


  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /** Get an accessor to the buffer with the required mode

      \param Mode is the requested access mode

      \param Target is the type of object to be accessed

      \todo Do we need for an accessor to increase the reference count of
      a buffer object? It does make more sense for a host-side accessor.

      \todo Implement the modes and targets
  */
  template <access::mode Mode,
            access::target Target = access::global_buffer>
  accessor<T, Dimensions, Mode, Target> get_access() const {
    return *implementation;
  }


  /// Get the range<> of the buffer
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection

       \todo Move into detail::

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
     */
    return range<Dimensions> { *(const std::size_t (*)[Dimensions])(implementation->allocation.shape()) };
  }


  /** Ask for read-only status of the buffer

      \todo Add to specification
  */
  bool is_read_only() const { return implementation->read_only; }


  /** Return the use count of the data of this buffer

      \todo Add to the specification? At least useful for the
      non-regression testing.
  */
  auto use_count() const {
    // Rely on the shared_ptr<> use_count()
    return implementation.use_count();
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

#endif // TRISYCL_SYCL_BUFFER_HPP
