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


  /** Create a new buffer of the given size with
      storage managed by the SYCL runtime

      The default behavior is to use the default host buffer
      allocator, in order to allow for host accesses. If the type of
      the buffer, has the const qualifier, then the default allocator
      will remove the qualifier to allow host access to the data.

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime
  */
  buffer(const range<Dimensions> &r, Allocator allocator = {})
    : implementation { new detail::buffer<T, Dimensions> { r } } {}


  /** Create a new buffer with associated host memory

      \param[in] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      cl::sycl::buffer_allocator<T> by default

      The host address is const T, so the host accesses can be
      read-only.

      However, the typename T is not const so the device accesses can
      be both read and write accesses. Since, the host_data is const,
      this buffer is only initialized with this memory and there is
      no write after its destruction, unless there is another final
      data address given after construction of the buffer.
  */
  buffer(const T *host_data,
         const range<Dimensions> &r,
         Allocator allocator = {})
    : implementation { new detail::buffer<T, Dimensions> { host_data, r } } {}


  /** Create a new buffer with associated host memory

      \param[inout] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      cl::sycl::buffer_allocator<T> by default

      The memory is owned by the runtime during the lifetime of the
      object.  Data is copied back to the host unless the user
      overrides the behavior using the set_final_data method. host_data
      points to the storage and values used by the buffer and
      range<dimensions> defines the size.
  */
  buffer(T *host_data, const range<Dimensions> &r, Allocator allocator = {})
    : implementation { new detail::buffer<T, Dimensions> { host_data, r } } {}


  /** Create a new buffer with associated memory, using the data in
      host_data

      \param[inout] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      cl::sycl::buffer_allocator<T> by default

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a cl::sycl::mutex_class is
      used. The mutex m is locked by the runtime whenever the data is in
      use and unlocked otherwise. Data is synchronized with host_data, when
      the mutex is unlocked by the runtime.

      \todo update the specification to replace the pointer by a
      reference and provide the constructor with and without a mutex
  */
  buffer(shared_ptr_class<T> &host_data,
         const range<Dimensions> &buffer_range,
         cl::sycl::mutex_class &m,
         Allocator allocator = {}) {
    detail::unimplemented();
  }


  /** Create a new buffer with associated memory, using the data in
      host_data

      \param[inout] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[inout] m is the mutex used to protect the data access

      \param[in] allocator is to be used by the SYCL runtime, of type
      cl::sycl::buffer_allocator<T> by default

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a cl::sycl::mutex_class is
      used.

      \todo add this mutex-less constructor to the specification
  */
  buffer(shared_ptr_class<T> &host_data,
         const range<Dimensions> &buffer_range,
         Allocator allocator = {})
    : implementation {
    new detail::buffer<T, Dimensions> { host_data, buffer_range } }
  {}


  /** Create a new buffer which is initialized by host_data

      \param[inout] host_data points to the storage and values used to
      initialize the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      cl::sycl::buffer_allocator<T> by default

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
         const range<Dimensions> &buffer_range,
         Allocator allocator = {})
  // Just delegate to the constructor with normal pointer
    : buffer(host_data.get(), buffer_range, allocator) {
    // Then release the host_data memory
    host_data.release();
  }


  /**  Create a new allocated 1D buffer initialized from the given
       elements ranging from first up to one before last

       The data is copied to an intermediate memory position by the
       runtime. Data is written back to the same iterator set if the
       iterator is not a const iterator.

      \param[inout] start_iterator points to the first element to copy

      \param[in] end_iterator points to just after the last element to copy

      \param[in] allocator is to be used by the SYCL runtime, of type
      cl::sycl::buffer_allocator<T> by default

      \todo Implement the copy back at buffer destruction

      \todo Generalize this for n-D and provide column-major and row-major
      initialization

      \todo a reason to have this nD is that
            set_final_data(weak_ptr_class<T> & finalData) is actually
            doing this linearization anyway

      \todo Allow read-only buffer construction too

      \todo update the specification to deal with forward iterators
      instead and rewrite back only when it is non const and output
      iterator at least

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


  /** Create a new sub-buffer without allocation to have separate
      accessors later

      \param[inout] b is the buffer with the real data

      \param[in] base_index specifies the origin of the sub-buffer inside the
      buffer b

      \param[in] sub_range specifies the size of the sub-buffer

      \todo To be implemented

      \todo Update the specification to replace index by id
  */
  buffer(buffer<T, Dimensions, Allocator> &b,
         const id<Dimensions> &base_index,
         const range<Dimensions> &sub_range,
         Allocator allocator = {}) { detail::unimplemented(); }


#ifdef TRISYCL_OPENCL
  /** Create a buffer from an existing OpenCL memory object associated
      with a context after waiting for an event signaling the
      availability of the OpenCL data

      \param[inout] mem_object is the OpenCL memory object to use

      \param[inout] from_queue is the queue associated to the memory
      object

      \param[in] available_event specifies the event to wait for if
      non null

      Note that a buffer created from a cl_mem object will only have
      one underlying cl_mem for the lifetime of the buffer and use on
      an incompatible queue constitues an error.

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

      \param[in] command_group_handler is the command group handler in
      which the kernel is to be executed

      \todo Do we need for an accessor to increase the reference count of
      a buffer object? It does make more sense for a host-side accessor.

      \todo Implement the modes and targets

      \todo for this implementation it is const for now
  */
  template <access::mode Mode,
            access::target Target = access::global_buffer>
  accessor<T, Dimensions, Mode, Target>
  get_access(handler &command_group_handler) const {
    static_assert(Target == access::global_buffer
                  || Target == access::constant_buffer,
                  "get_access(handler) can only deal with access::global_buffer"
                  " or access::constant_buffer (for host_buffer accessor"
                  " do not use a command group handler");
    return *implementation;
  }


  /** Get a host accessor to the buffer with the required mode

      \param Mode is the requested access mode

      \todo Implement the modes

      \todo More elegant solution

      \todo for this implementation it is const for now
  */
  template <access::mode Mode,
            access::target Target = access::host_buffer>
  accessor<T, Dimensions, Mode, Target>
  get_access() const {
    static_assert(Target == access::host_buffer,
                  "get_access() without a command gtoup handler is only "
                  " for host_buffer accessor");
    return *implementation;
  }


  /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor

      \todo rename to the equivalent from array_view proposals? Such
      as size() in
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0009r0.html
      or
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0114r0.pdf
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection
     */
    return implementation->get_range();
  }


  /** Returns the total number of elements in the buffer

      Equal to get_range()[0] * ... * get_range()[dimensions-1].
  */
  auto get_count() const {
    return implementation->get_count();
  }


  /** Returns the size of the buffer storage in bytes

      Equal to get_count()*sizeof(T).

      \todo rename to something else. In
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0122r0.pdf
      it is named bytes() for example
  */
  size_t get_size() const {
    return get_count()*sizeof(T);
  }


  /** Returns the number of buffers that are shared/referenced

      For example
      \code
      cl::sycl::buffer<int> b { 1000 };
      // Here b.use_count() should return 1
      cl::sycl::buffer<int> c { b };
      // Here b.use_count() and b.use_count() should return 2
      \endcode

      \todo Add to the specification, useful for validation
  */
  auto use_count() const {
    // Rely on the shared_ptr<> use_count()
    return implementation.use_count();
  }


  /** Ask for read-only status of the buffer

      \todo Add to specification
  */
  bool is_read_only() const { return implementation->read_only; }


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

      \todo figure out how set_final_data() interact with the other
      way to write back some data or with some data sharing with the
      host that can not be undone
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
