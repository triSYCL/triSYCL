#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP

/** \file The OpenCL SYCL buffer<> detail implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>

// \todo Use C++17 optional when it is mainstream
#include <boost/optional.hpp>

#include "triSYCL/access.hpp"
#include "triSYCL/accessor/mixin/accessor.hpp"
#include "triSYCL/buffer/detail/accessor.hpp"
#include "triSYCL/buffer/detail/buffer_base.hpp"
#include "triSYCL/buffer/detail/buffer_waiter.hpp"
#include "triSYCL/range.hpp"

namespace trisycl::detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T, int Dimensions = 1>
class buffer
    : public detail::buffer_base
    , public mixin::accessor<T, Dimensions>
    , public detail::debug<buffer<T, Dimensions>> {
 private:
  // To access directly some accessor aspects here
  using mixin = mixin::accessor<T, Dimensions>;

  // \todo Replace U and D somehow by T and Dimensions
  // To allow allocation access
  template <typename U, int D, access::mode Mode,
            access::target Target /* = access::global_buffer */>
  friend class detail::accessor;

  /** The allocator to be used when some memory is needed

      \todo Implement user-provided allocator
  */
  std::allocator<typename mixin::value_type> alloc;

  /** If some allocation is requested on the host for the buffer
      memory, this is where the memory is attached to.

      Note that this is uninitialized memory, as stated in SYCL
      specification.
  */
  typename mixin::pointer allocation = nullptr;

  /** How to copy back data on buffer destruction, can be modified with
      set_final_data( ... )
  */
  boost::optional<std::function<void(void)>> final_write_back;

  /// Keep the shared pointer used to create the buffer
  shared_ptr_class<T> input_shared_pointer;

  /// Track if the buffer memory is backed by user-provided host memory
  bool data_host = false;

  /** Track if data should be copied into buffer-owned memory if a
      modification occurs */
  bool copy_if_modified = false;

  // Track if data have been modified
  bool modified = false;

  /// Track the host context
  trisycl::context host_context { trisycl::device {} };

 public:
  /// Create a new read-write buffer of size \param r
  buffer(const range<Dimensions>& r)
      /// \todo Lazily allocate memory since it might not be used on host
      : mixin { allocate_buffer(r), r } {}

  /** Create a new read-write buffer from \param host_data of size
      \param r without further allocation */
  buffer(T* host_data, const range<Dimensions>& r)
      : mixin { host_data, r }
      , data_host { true } {}

  /** Create a new read-only buffer from \param host_data of size \param r
      without further allocation

      If the buffer is non const, use a copy-on-write mechanism with
      internal writable memory.

      \todo Clarify the semantics in the spec. What happens if the
      host change the host_data after buffer creation?

      Only enable this constructor if the value type is not constant,
      because if it is constant, the buffer is constant too.
  */
  template <typename Dependent = T,
            typename = std::enable_if_t<!std::is_const<Dependent>::value>>
  buffer(const T* host_data, const range<Dimensions>& r)
      : /* The buffer is read-only, even if the internal multidimensional
           wrapper is not. If a write accessor is requested, there should
           be a copy on write. So this pointer should not be written and
           this const_cast should be acceptable. */
      mixin { const_cast<T*>(host_data), r }
      , data_host { true }
      ,
      /* Set copy_if_modified to true, so that if an accessor with write
         access is created, data are copied before to be modified. */
      copy_if_modified { true } {}

  /** Create a new buffer with associated memory, using the data in
      host_data

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a trisycl::mutex_class is
      used.
  */
  buffer(shared_ptr_class<T>& host_data, const range<Dimensions>& r)
      : mixin { host_data.get(), r }
      , input_shared_pointer { host_data }
      , data_host { true } {}

  /// Create a new allocated 1D buffer from the given elements
  template <typename Iterator>
  buffer(Iterator start_iterator, Iterator end_iterator)
      : mixin { allocate_buffer(std::distance(start_iterator, end_iterator)),
                range<1> { static_cast<std::size_t>(
                    std::distance(start_iterator, end_iterator)) } } {
    /* Then assign allocation since this is the only multi_array
       method with this iterator interface */
    assign(start_iterator, end_iterator);
  }

  /** Create a new sub-buffer without allocation to have separate
      accessors later

      \todo To implement and deal with reference counting
  buffer(buffer<T, Dimensions> b,
         index<Dimensions> base_index,
         range<Dimensions> sub_range)
  */

  /// \todo Allow CLHPP objects too?
  ///
  /*
  buffer(cl_mem mem_object,
         queue from_queue,
         event available_event)
  */

  /** The buffer content may be copied back on destruction to some
      final location */
  ~buffer() {
#ifdef TRISYCL_OPENCL
    /* We ensure that the host has the most up-to-date version of the data
       before the buffer is destroyed. This is necessary because we do not
       systematically transfer the data back from a device with
       \c copy_back_cl_buffer any more.
       \todo Optimize for the case the buffer is not based on host memory
    */
    call_update_buffer_state(host_context, access::mode::read,
                             mixin::get_size(), mixin::access.data());

#endif
    if (modified && final_write_back)
      (*final_write_back)();
    // Allocate explicitly allocated memory if required
    deallocate_buffer();
  }

  /** Enforce the buffer to be considered as being modified.
      Same as creating an accessor with write access.
   */
  void mark_as_written() { modified = true; }

  /** This method is to be called whenever an accessor is created

      Its current purpose is to track if an accessor with write access
      is created and acting accordingly.
   */
  template <access::mode Mode,
            access::target Target = access::target::host_buffer>
  void track_access_mode() {
    // test if write access is required
    if (Mode == access::mode::write || Mode == access::mode::read_write ||
        Mode == access::mode::discard_write ||
        Mode == access::mode::discard_read_write ||
        Mode == access::mode::atomic) {
      modified = true;
      if (copy_if_modified) {
        // Implement the allocate & copy-on-write optimization
        copy_if_modified = false;
        // Since the mixin store the geometry, keep a copy before updating
        auto current_access = mixin::access;
        /* The range is actually computed from \c access itself, so
           save it */
        auto current_range = mixin::get_range();
        allocate_buffer(current_range);
        /* Update the mixin accessor to point to the new allocated
           memory instead */
        mixin::update(allocation, current_range);
        // Then copy the read-only data to the new allocated place
        std::uninitialized_copy_n(current_access.data(), mixin::get_count(),
                                  mixin::access.data());
        /* Now the data of the buffer is no longer backed-up by host
           user provided memory */
        data_host = false;
      }
    }
  }

  /** Set the weak pointer as destination for write-back on buffer
      destruction
  */
  void set_final_data(std::weak_ptr<T> final_data) {
    // Capture this by reference is enough since the buffer will still exist
    final_write_back = [this, final_data = std::move(final_data)] {
      if (auto sptr = final_data.lock()) {
        std::copy_n(mixin::access.data(), mixin::get_count(), sptr.get());
      }
    };
  }

  /** Disable write-back on buffer destruction as an iterator.
   */
  void set_final_data(std::nullptr_t) { final_write_back = boost::none; }

  /** Provide destination for write-back on buffer destruction as an
      iterator
  */
  template <
      typename Iterator,
      typename ValueType = typename std::iterator_traits<Iterator>::value_type>
  void set_final_data(Iterator final_data) {
    /*   using type_ = typename iterator_value_type<Iterator>::value_type;
         static_assert(std::is_same<type_, T>::value, "buffer type mismatch");
         static_assert(!(std::is_const<type_>::value),
                       "const iterator is not allowed");*/
    // Capture this by reference is enough since the buffer will still exist
    final_write_back = [this, final_data = std::move(final_data)] {
      std::copy_n(mixin::access.data(), mixin::get_count(), final_data);
    };
  }

 private:
  /// Allocate uninitialized buffer memory
  auto allocate_buffer(const range<Dimensions>& r) {
    auto count = r.size();
    // Allocate uninitialized memory
    allocation = alloc.allocate(count);
    return allocation;
  }

  /// Deallocate buffer memory if required
  void deallocate_buffer() {
    if (allocation)
      alloc.deallocate(allocation, mixin::get_count());
  }

  /** Assign the 1-D storage behind the accessor

      Use 2 different iterator types since in C++20 ranges it is now
      the case.
  */
  template <typename StartIter, typename EndIter>
  void assign(StartIter start_iterator, EndIter end_iterator) {
    std::copy(start_iterator, end_iterator, mixin::access.data());
  }

  /** Function pair to work around the fact that T might be a \c const type.
      We call update_buffer_state only if T is not \c const, we have to
      use \c enable_if otherwise the compiler will try to cast \c const
      \c void* to \c void* if we create a buffer with a \c const type

      \todo Use \c if \c constexpr when it is available with C++17
  */
  template <typename BaseType = T, typename DataType>
  void call_update_buffer_state(
      trisycl::context ctx, access::mode mode, size_t size, DataType* data,
      std::enable_if_t<!std::is_const<BaseType>::value>* = 0) {
    update_buffer_state(ctx, mode, size, data);
  }

  /** Version of \c call_update_buffer_state that does nothing. It is called if
      the type of the data in the buffer is \c const
   */
  template <typename BaseType = T, typename DataType>
  void call_update_buffer_state(
      trisycl::context ctx, access::mode mode, size_t size, DataType* data,
      std::enable_if_t<std::is_const<BaseType>::value>* = 0) {}

 public:
  /** Get a \c future to wait from inside the \c trisycl::buffer in
      case there is something to copy back to the host

      \return A \c future in the \c optional if there is something to
      wait for, otherwise an empty \c optional
      \todo Make the function private again
  */
  boost::optional<std::future<void>> get_destructor_future() {
    /* If there is only 1 shared_ptr user of the buffer, this is the
       caller of this function, the \c buffer_waiter, so there is no
       need to get a \ future otherwise there will be a dead-lock if
       there is only 1 thread waiting for itself.

       Since \c use_count() is applied to a \c shared_ptr just created
       for this purpose, it actually increase locally the count by 1,
       so check for 1 + 1 use count instead...
    */
    // If the buffer's destruction triggers a write-back, wait
    if ((shared_from_this().use_count() > 2) && modified &&
        (final_write_back || data_host)) {
      // Create a promise to wait for
      notify_buffer_destructor = std::promise<void> {};
      // And return the future to wait for it
      return notify_buffer_destructor->get_future();
    }
    return boost::none;
  }

 private:
  // Allow buffer_waiter destructor to access get_destructor_future()
  // friend detail::buffer_waiter<T, Dimensions>::~buffer_waiter();
  /* \todo Work around to Clang bug
     https://llvm.org/bugs/show_bug.cgi?id=28873 cannot use destructor
     here */
  /* \todo solve the fact that get_destructor_future is not accessible
     when private and buffer_waiter uses a custom allocator */
  friend detail::buffer_waiter<T, Dimensions>;
};

/** Proxy function to avoid some circular type recursion

    \return a shared_ptr<task>

    \todo To remove with some refactoring
*/
template <typename BufferDetail>
static std::shared_ptr<detail::task>
buffer_add_to_task(BufferDetail buf, handler* command_group_handler,
                   bool is_write_mode) {
  return buf->add_to_task(command_group_handler, is_write_mode);
}

/// @} End the data Doxygen group

} // namespace trisycl::detail

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
