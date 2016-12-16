#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP

/** \file The OpenCL SYCL buffer<> detail implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include <boost/multi_array.hpp>
// \todo Use C++17 optional when it is mainstream
#include <boost/optional.hpp>

#include "CL/sycl/access.hpp"
#include "CL/sycl/buffer/detail/accessor.hpp"
#include "CL/sycl/buffer/detail/buffer_base.hpp"
#include "CL/sycl/buffer/detail/buffer_waiter.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T,
          std::size_t Dimensions = 1>
class buffer : public detail::buffer_base,
               public detail::debug<buffer<T, Dimensions>> {
public:

  // Extension to SYCL: provide pieces of STL container interface
  using element = T;
  using value_type = T;

private:

  /** If some allocation is requested, it is managed by this multi_array
      to ease initialization from data */
  boost::multi_array<T, Dimensions> allocation;

  // \todo Replace U and D somehow by T and Dimensions
  // To allow allocation access
  template <typename U,
            std::size_t D,
            access::mode Mode,
            access::target Target /* = access::global_buffer */>
    friend class detail::accessor;


  /** This is the multi-dimensional interface to the data that may point
      to either allocation in the case of storage managed by SYCL itself
      or to some other memory location in the case of host memory or
      storage<> abstraction use
  */
  boost::multi_array_ref<T, Dimensions> access;

  /// The weak pointer to copy back data on buffer deletion
  weak_ptr_class<T> final_data;

  /** The shared pointer in the case the buffer memory is shared with
      the host */
  shared_ptr_class<T> shared_data;

  // Track if the buffer memory is provided as host memory
  bool host_write_back = false;

public:

  /// Create a new read-write buffer of size \param r
  buffer(const range<Dimensions> &r) : buffer_base { false },
                                       allocation { r },
                                       access { allocation }
                                       {}


  /** Create a new read-write buffer from \param host_data of size
      \param r without further allocation */
  buffer(T *host_data, const range<Dimensions> &r) : buffer_base { false },
                                                     access { host_data, r },
                                                     host_write_back { true }
                                                     {}


  /** Create a new read-only buffer from \param host_data of size \param r
      without further allocation

      \todo Clarify the semantics in the spec. What happens if the
      host change the host_data after buffer creation?
  */
  buffer(const T *host_data, const range<Dimensions> &r) :
    /* \todo Need to solve this const buffer issue in a clean way

       Just allocate memory? */
    buffer_base { true },
    access { const_cast<T *>(host_data), r }
    {}


  /** Create a new buffer with associated memory, using the data in
      host_data

      The ownership of the host_data is shared between the runtime and the
      user. In order to enable both the user application and the SYCL
      runtime to use the same pointer, a cl::sycl::mutex_class is
      used.
  */
  buffer(shared_ptr_class<T> &host_data,
         const range<Dimensions> &r)
    : buffer_base { false },
    access { host_data.get(), r },
    shared_data { host_data }
    {}


  /// Create a new allocated 1D buffer from the given elements
  template <typename Iterator>
  buffer(Iterator start_iterator, Iterator end_iterator) :
    buffer_base { false },
    // The size of a multi_array is set at creation time
    allocation { boost::extents[std::distance(start_iterator, end_iterator)] },
    access { allocation }
    {
      /* Then assign allocation since this is the only multi_array
         method with this iterator interface */
      allocation.assign(start_iterator, end_iterator);
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
    /* If there is a final_data set and that points to something
       alive, copy back the data through the shared pointer */
    if (auto p = final_data.lock())
      std::copy_n(access.data(), access.num_elements(), p.get());
    /* If data are shared with the host but not concretely, we would
       have to copy back the data to the host */
    // else if (shared_data)
    //   std::copy_n(access.data(), access.num_elements(), shared_data.get());
  }

  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /// Return an accessor of the required mode \param M
  /// \todo Remove if not used
  template <access::mode Mode,
            access::target Target = access::target::global_buffer>
  detail::accessor<T, Dimensions, Mode, Target> get_access() {
    return { *this };
  }


 /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
    */
    return range<Dimensions> {
      *(const std::size_t (*)[Dimensions])(allocation.shape())
        };
  }


  /** Returns the total number of elements in the buffer

      Equal to get_range()[0] * ... * get_range()[dimensions-1].
  */
  auto get_count() const {
    return access.num_elements();
  }


  /** Returns the size of the buffer storage in bytes

      \todo rename to something else. In
      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0122r0.pdf
      it is named bytes() for example
  */
  auto get_size() const {
    return get_count()*sizeof(value_type);
  }


  /** Set the weak pointer to copy back data on buffer deletion

      \todo Add a write kernel dependency on the buffer so the buffer
      destructor has to wait for the kernel execution if the buffer is
      also accessed through a write accessor
  */
  void set_final_data(weak_ptr_class<T> && finalData) {
    final_data = finalData;
  }

private:

  /** Get a \c future to wait from inside the \c cl::sycl::buffer in
      case there is something to copy back to the host

      \return A \c future in the \c optional if there is something to
      wait for, otherwise an empty \c optional
  */
  boost::optional<std::future<void>> get_destructor_future() {
    boost::optional<std::future<void>> f;
    /* If there is only 1 shared_ptr user of the buffer, this is the
       caller of this function, the \c buffer_waiter, so there is no
       need to get a \ future otherwise there will be a dead-lock if
       there is only 1 thread waiting for itself.

       Since \c use_count() is applied to a \c shared_ptr just created
       for this purpose, it actually increase locally the count by 1,
       so check for 1 + 1 use count instead...
    */
    if (shared_from_this().use_count() > 2)
      // \todo Double check the specification and add unit tests
      if (host_write_back || !final_data.expired() || shared_data) {
        // Create a promise to wait for
        notify_buffer_destructor = std::promise<void> {};
        // And return the future to wait for it
        f = notify_buffer_destructor->get_future();
      }
    return f;
  }


  // Allow buffer_waiter destructor to access get_destructor_future()
  // friend detail::buffer_waiter<T, Dimensions>::~buffer_waiter();
  /* \todo Work around to Clang bug
     https://llvm.org/bugs/show_bug.cgi?id=28873 cannot use destructor
     here */
  friend detail::buffer_waiter<T, Dimensions>;

};


/** Proxy function to avoid some circular type recursion

    \return a shared_ptr<task>

    \todo To remove with some refactoring
*/
template <typename BufferDetail>
static std::shared_ptr<detail::task>
buffer_add_to_task(BufferDetail buf,
                   handler *command_group_handler,
                   bool is_write_mode) {
    return buf->add_to_task(command_group_handler, is_write_mode);
  }

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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
