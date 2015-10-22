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
#include "CL/sycl/detail/global_config.hpp"
#include "CL/sycl/event.hpp"
#include "CL/sycl/handler.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/queue.hpp"
#include "CL/sycl/range.hpp"

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

    \todo There is a naming inconsistency in the specification between
    buffer and accessor on T versus datatype

    \todo Think about the need of an allocator when constructing a buffer
    from other buffers
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

  /** Create a new buffer with associated memory, using the data in
      host_data

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a cl::sycl::mutex_class is
      used. The mutex m is locked by the runtime whenever the data is in
      use and unlocked otherwise. Data is synchronized with host_data, when
      the mutex is unlocked by the runtime.
  */
  buffer(shared_ptr_class<T> & host_data,
         const range<Dimensions> & buffer_range,
         cl::sycl::mutex_class * m = nullptr,
         Allocator allocator = {}) {
    detail::unimplemented();
  }


  /** Create a new buffer which is initialized by host_data

      The SYCL runtime receives full ownership of the host_data unique_ptr
      and there in effect there is no synchronization with the application
      code using host_data.

      \todo Update the API to add template <typename D =
      std::default_delete<T>> because the
      unique_ptr_class/std::unique_ptr have the destructor type as
      dependent
  */
  template <typename D = std::default_delete<T>>
  buffer(unique_ptr_class<T, D> &&host_data,
         const range<Dimensions> & buffer_range,
         Allocator allocator = {})
  // Just delegate to the constructor with normal pointer
    : buffer(host_data.get(), buffer_range, allocator) {
    // Then release the host_data memory
    host_data.release();
  }


  /** Create a new read-write allocated 1D buffer initialized from the
      given elements

      \param start_iterator points to the first element to copy

      \param end_iterator points to just after the last element to copy

      \todo Add const to the SYCL specification.

      \todo Generalize this for n-D and provide column-major and row-major
      initialization

      \todo a reason to have this nD is that
            set_final_data(weak_ptr_class<T> & finalData) is actually
            doing this linearization anyway

      \todo Allow read-only buffer construction too

      \todo Allow initialization from ranges and collections à la STL
  */
  template <typename InputIterator,
            /* To force some iterator concept checking to avoid GCC 4.9
               diving into this when initializing from ({ int, int })
               which is a range<> and and not an iterator... */
            typename ValueType =
            typename std::iterator_traits<InputIterator>::value_type>
  buffer(InputIterator start_iterator,
         InputIterator end_iterator,
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
         Allocator allocator = {}) { detail::unimplemented(); }


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
         event available_event = {},
         Allocator allocator = {}) { detail::unimplemented();  }
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
  accessor<T, Dimensions, Mode, Target>
  get_access(handler &command_group_handler) const {
    static_assert(Target != access::host_buffer,
                  "get_access(&cgh) for non host_buffer accessor "
                  "takes a command group handler");
    return *implementation;
  }


  /** Get a host accessor to the buffer with the required mode

      \param Mode is the requested access mode

      \todo Implement the modes

      \todo More elegant solution
  */
  template <access::mode Mode,
            access::target Target = access::global_buffer>
  accessor<T, Dimensions, Mode, access::host_buffer>
  get_access() const {
    static_assert(Target == access::host_buffer,
                  "get_access() for host_buffer accessor does not "
                  "take a command group handler");
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

      \todo Rename to use_count() to follow shared_ptr<> naming
  */
  auto get_count() const {
    // Rely on the shared_ptr<> use_count()
    return implementation.use_count();
  }


  /** Set destination of buffer data on destruction

      The finalData points to the host memory to which, the outcome of all
      the buffer processing is going to be copied to.

      This is the final pointer, which is going to be accessible after the
      destruction of the buffer and in the case where this is a valid
      pointer, the data are going to be copied to this host address.

      finalData is different from the original host address, if the buffer
      was created associated with one. This is mainly to be used when a
      shared_ptr is given in the constructor and the output data will
      reside in a different location from the initialization data.

      It is defined as a weak_ptr referring to a shared_ptr that is not
      associated with the cl::sycl::buffer, and so the cl::sycl::buffer
      will have no ownership of finalData.

      \todo Update the API to take finalData by value instead of by
            reference.  This way we can have an implicit conversion
            possible at the API call from a shared_ptr<>, avoiding an
            explicit weak_ptr<> creation
  */
  void set_final_data(weak_ptr_class<T> finalData) {
    implementation->set_final_data(std::move(finalData));
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
