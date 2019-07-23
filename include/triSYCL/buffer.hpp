#ifndef TRISYCL_SYCL_BUFFER_HPP
#define TRISYCL_SYCL_BUFFER_HPP

/** \file The OpenCL SYCL buffer<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

#include "triSYCL/access.hpp"
#include "triSYCL/accessor.hpp"
#include "triSYCL/buffer/detail/buffer.hpp"
#include "triSYCL/buffer/detail/buffer_waiter.hpp"
#include "triSYCL/buffer_allocator.hpp"
#include "triSYCL/detail/global_config.hpp"
#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/event.hpp"
#include "triSYCL/handler.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/queue.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    \todo There is a naming inconsistency in the specification between
    buffer and accessor on T versus datatype

    \todo Finish allocator implementation

    \todo Think about the need of an allocator when constructing a buffer
    from other buffers

    \todo Update the specification to have a non-const allocator for
    const buffer? Or do we rely on rebind_alloc<T>. But does this work
    with astate-full allocator?

    \todo Add constructors from arrays so that in C++17 the range and
    type can be infered from the constructor

    \todo Add constructors from array_ref
*/
template <typename T,
          int Dimensions = 1,
          /* Even a buffer of const T may need to allocate memory, so
             need an allocator of non const T */
          typename Allocator = buffer_allocator<std::remove_const_t<T>>>
class buffer
  /* Use the underlying buffer waiter implementation that can be
     shared in the SYCL model */
  : public detail::shared_ptr_implementation<
                         buffer<T, Dimensions, Allocator>,
                         detail::buffer_waiter<T, Dimensions, Allocator>>,
    detail::debug<buffer<T, Dimensions, Allocator>> {
public:

  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using allocator_type = Allocator;

private:

  // The type encapsulating the implementation
  using implementation_t = typename buffer::shared_ptr_implementation;

  // Allows the comparison operation to access the implementation
  friend implementation_t;

public:

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  /** Use default constructors so that we can create a new buffer copy
      from another one, with either a l-value or an r-value (for
      std::move() for example).

      Since we just copy the shared_ptr<> from the
      shared_ptr_implementation above, this is where/how the sharing
      magic is happening with reference counting in this case.
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
    : implementation_t { detail::waiter<T, Dimensions, Allocator>(
                         new detail::buffer<T, Dimensions> { r }) }
      {}


  /** Create a new buffer with associated host memory

      \param[in] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      \c trisycl::buffer_allocator<T> by default

      The host address is \code const T* \endcode, so the host memory
      is read-only.

      However, the typename T is not const so the device accesses can
      be both read and write accesses. Since, the host_data is const,
      this buffer is only initialized with this memory and there is
      no write after its destruction, unless there is another final
      data address given after construction of the buffer.

      Only enable this constructor if it is not the same as the one
      with \code const T *host_data \endcode, which is when \c T is
      already a constant type.

      \todo Actually this is redundant.
  */
  template <typename Dependent = T,
            typename = std::enable_if_t<!std::is_const<Dependent>::value>>
  buffer(const T *host_data,
         const range<Dimensions> &r,
         Allocator allocator = {})
    : implementation_t { detail::waiter(new detail::buffer<T, Dimensions>
                         { host_data, r }) }
  {}


  /** Create a new buffer with associated host memory

      \param[inout] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      trisycl::buffer_allocator<T> by default

      The memory is owned by the runtime during the lifetime of the
      object.  Data is copied back to the host unless the user
      overrides the behavior using the set_final_data method. host_data
      points to the storage and values used by the buffer and
      range<Dimensions> defines the size.
  */
  buffer(T *host_data,
         const range<Dimensions> &r,
         Allocator allocator = {})
    : implementation_t { detail::waiter(new detail::buffer<T, Dimensions>
                         { host_data, r }) }
  {}


  /** Create a new buffer with associated memory, using the data in
      host_data

      \param[inout] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[in] allocator is to be used by the SYCL runtime, of type
      trisycl::buffer_allocator<T> by default

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a trisycl::mutex_class is
      used. The mutex m is locked by the runtime whenever the data is in
      use and unlocked otherwise. Data is synchronized with host_data, when
      the mutex is unlocked by the runtime.

      \todo update the specification to replace the pointer by a
      reference and provide the constructor with and without a mutex
  */
  buffer(shared_ptr_class<T> &host_data,
         const range<Dimensions> &buffer_range,
         trisycl::mutex_class &m,
         Allocator allocator = {}) {
    TRISYCL_UNIMPL;
  }


  /** Create a new buffer with associated memory, using the data in
      host_data

      \param[inout] host_data points to the storage and values used by
      the buffer

      \param[in] r defines the size

      \param[inout] m is the mutex used to protect the data access

      \param[in] allocator is to be used by the SYCL runtime, of type
      trisycl::buffer_allocator<T> by default

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a trisycl::mutex_class is
      used.

      \todo add this mutex-less constructor to the specification
  */
  buffer(shared_ptr_class<T> host_data,
         const range<Dimensions> &buffer_range,
         Allocator allocator = {})
    : implementation_t { detail::waiter(new detail::buffer<T, Dimensions>
                         { host_data, buffer_range }) }
  {}


  /** Create a new allocated 1D buffer initialized from the given
      elements ranging from first up to one before last

      The data is copied to an intermediate memory position by the
      runtime. Data is written back to the same iterator set if the
      iterator is not a const iterator.

      \param[inout] start_iterator points to the first element to copy

      \param[in] end_iterator points to just after the last element to copy

      \param[in] allocator is to be used by the SYCL runtime, of type
      trisycl::buffer_allocator<T> by default

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
    implementation_t { detail::waiter(new detail::buffer<T, Dimensions>
                       { start_iterator, end_iterator }) }
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
         Allocator allocator = {}) { TRISYCL_UNIMPL; }


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
         Allocator allocator = {}) { TRISYCL_UNIMPL;  }
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
  */
  template <access::mode Mode,
            access::target Target = access::target::global_buffer>
  accessor<T, Dimensions, Mode, Target>
  get_access(handler &command_group_handler) {
    static_assert(Target == access::target::global_buffer
                  || Target == access::target::constant_buffer,
                  "get_access(handler) can only deal with access::global_buffer"
                  " or access::constant_buffer (for host_buffer accessor"
                  " do not use a command group handler");
    implementation->implementation->template track_access_mode<Mode, Target>();
    return { *this, command_group_handler };
  }


  /** Force the buffer to behave like if we had created
      an accessor in write mode.
   */
  void mark_as_written() {
    return implementation->implementation->mark_as_written();
  }


  /** Get a host accessor to the buffer with the required mode

      \param Mode is the requested access mode

      \todo Implement the modes

      \todo More elegant solution
  */
  template <access::mode Mode>
  accessor<T, Dimensions, Mode, access::target::host_buffer>
  get_access() {
    implementation->implementation->template track_access_mode<Mode, access::target::host_buffer>();
    return { *this };
  }


  /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor

      \todo rename to the equivalent from array_ref proposals? Such
      as size() in
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0009r2.html
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection
    */
    return implementation->implementation->get_range();
  }


  /** Returns the total number of elements in the buffer

      Equal to get_range()[0] * ... * get_range()[Dimensions-1].
  */
  auto get_count() const {
    return implementation->implementation->get_count();
  }


  /** Returns the size of the buffer storage in bytes

      Equal to get_count()*sizeof(T).

      \todo rename to something else. In
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0122r0.pdf
      it is named bytes() for example
  */
  size_t get_size() const {
    return implementation->implementation->get_size();
  }


  /** Returns the number of buffers that are shared/referenced

      For example
      \code
      trisycl::buffer<int> b { 1000 };
      // Here b.use_count() should return 1
      trisycl::buffer<int> c { b };
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
  bool constexpr is_read_only() const {
    return std::is_const<T>::value;
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
      associated with the trisycl::buffer, and so the trisycl::buffer
      will have no ownership of finalData.
  */
  void set_final_data(weak_ptr_class<T> finalData) {
    implementation->implementation->set_final_data(std::move(finalData));
  }


  /** Disable write-back on buffer destruction.
   */
  void set_final_data(std::nullptr_t) {
    implementation->implementation->set_final_data(nullptr);
  }


#ifdef TRISYCL_OPENCL
  /** Check if the buffer is already cached in a certain context
   */
  bool is_cached(trisycl::context& ctx) {
    return implementation->implementation->is_cached(ctx);
  }


  /** Check if the data stored in the buffer is up-to-date in a certain context
   */
  bool is_data_up_to_date(trisycl::context& ctx) {
    return implementation->implementation->is_data_up_to_date(ctx);
  }
#endif

  /** Set destination of buffer data on destruction.

      WARNING: the user has to ensure that the object referred to by the
      iterator will be alive after buffer destruction, otherwise the behavior
      is undefined.
   */
  template <typename Iterator,
            typename ValueType =
            typename std::iterator_traits<
              std::remove_reference_t<Iterator>>::value_type>
  void set_final_data(Iterator &&finalData) {
    implementation->implementation->set_final_data(
      std::forward<Iterator>(finalData));
  }

};

/// @} End the data Doxygen group

}

/* Inject a custom specialization of std::hash to have the buffer
   usable into an unordered associative container

   \todo Add this to the spec
*/
namespace std {

template <typename T,
          int Dimensions,
          typename Allocator>
struct hash<trisycl::buffer<T, Dimensions, Allocator>> {

  auto operator()(const trisycl::buffer<T, Dimensions, Allocator> &b) const {
    // Forward the hashing to the implementation
    return b.hash();
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

#endif // TRISYCL_SYCL_BUFFER_HPP
