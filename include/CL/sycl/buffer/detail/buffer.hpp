#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP

/** \file The OpenCL SYCL buffer<> detail implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include <boost/multi_array.hpp>

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor/detail/accessor.hpp"
#include "CL/sycl/buffer/detail/buffer_base.hpp"
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
struct buffer : public detail::debug<buffer<T, Dimensions>>,
                public detail::buffer_base {
  // Extension to SYCL: provide pieces of STL container interface
  using element = T;
  using value_type = T;

  /** If some allocation is requested, it is managed by this multi_array
      to ease initialization from data */
  boost::multi_array<T, Dimensions> allocation;

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

  /// Create a new read-write buffer of size \param r
  buffer(const range<Dimensions> &r) : buffer_base { false },
                                       allocation { r },
                                       access { allocation }
                                       {}


  /** Create a new read-write buffer from \param host_data of size
      \param r without further allocation */
  buffer(T *host_data, const range<Dimensions> &r) : buffer_base { false },
                                                     access { host_data, r }
                                                     {}


  /** Create a new read-only buffer from \param host_data of size \param r
      without further allocation */
  buffer(const T *host_data, const range<Dimensions> &r) :
    /// \todo Need to solve this const buffer issue in a clean way
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
    shared_data { }
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
            access::target Target = access::global_buffer>
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
    return allocation.num_elements();
  }


  /** Set the weak pointer to copy back data on buffer deletion

      \todo Add a write kernel dependency on the buffer so the buffer
      destructor has to wait for the kernel execution if the buffer is
      also accessed through a write accessor
  */
  void set_final_data(weak_ptr_class<T> && finalData) {
    final_data = finalData;
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

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_HPP
