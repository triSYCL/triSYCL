/** \file

    This is a simple C++ sequential OpenCL SYCL implementation to
    experiment with the OpenCL CL provisional specification.

    Ronan.Keryell at AMD point com

    Copyright 2014 Advanced Micro Devices, Inc.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cassert>
#include <deque>
#include <functional>
#include <type_traits>
#include <iostream>
#include <iterator>
#include <boost/multi_array.hpp>

/// \todo Move into files really using it
#include "CL/sycl/detail/debug.hpp"

namespace cl {
namespace sycl {
namespace trisycl {

template <typename T,
          std::size_t dimensions,
          access::mode mode,
          access::target target = access::global_buffer>
struct AccessorImpl;
class Task;

}
}
}

/// triSYCL implementation dwells in the cl::sycl::trisycl namespace
namespace cl {
namespace sycl {

// Forward declaration for the buffer<> and the accessor<>
template <std::size_t dims> struct item;
template <std::size_t dims> struct nd_item;

namespace trisycl {

/** \addtogroup data
    @{
*/

/** A SYCL buffer is a multidimensional variable length array (à la C99
    VLA or even Fortran before) that is used to store data to work on.

    In the case we initialize it from a pointer, for now we just wrap the
    data with boost::multi_array_ref to provide the VLA semantics without
    any storage.
*/
template <typename T,
          std::size_t dimensions = 1>
struct BufferImpl : public detail::debug<BufferImpl<T, dimensions>>,
  detail::buffer_base {
  using Implementation = boost::multi_array_ref<T, dimensions>;
  // Extension to SYCL: provide pieces of STL container interface
  using element = T;
  using value_type = T;

  /** If some allocation is requested, it is managed by this multi_array
      to ease initialization from data */
  boost::multi_array<T, dimensions> Allocation;
  /** This is the multi-dimensional interface to the data that may point
      to either Allocation in the case of storage managed by SYCL itself
      or to some other memory location in the case of host memory or
      storage<> abstraction use
  */
  boost::multi_array_ref<T, dimensions> Access;


  /// Create a new read-write BufferImpl of size \param r
  BufferImpl(range<dimensions> const &r) : buffer_base(false),
                                           Allocation(r),
                                           Access(Allocation) {}


  /** Create a new read-write BufferImpl from \param host_data of size
      \param r without further allocation */
  BufferImpl(T * host_data, range<dimensions> r) : buffer_base(false),
                                                   Access(host_data, r) {}


  /** Create a new read-only BufferImpl from \param host_data of size \param r
      without further allocation */
  BufferImpl(const T * host_data, range<dimensions> r) :
    /// \todo Need to solve this const buffer issue in a clean way
    Access(const_cast<T *>(host_data), r),
    buffer_base(true) {}


  /// \todo
  //BufferImpl(storage<T> &store, range<dimensions> r)

  /// Create a new allocated 1D BufferImpl from the given elements
  template <typename Iterator>
  BufferImpl(Iterator start_iterator, Iterator end_iterator) :
    buffer_base(false),
    // The size of a multi_array is set at creation time
    Allocation(boost::extents[std::distance(start_iterator, end_iterator)]),
    Access(Allocation) {
    /* Then assign Allocation since this is the only multi_array
       method with this iterator interface */
    Allocation.assign(start_iterator, end_iterator);
  }


  /** Create a new BufferImpl from an old one, with a new allocation

      \todo Refactor the implementation to deal with buffer sharing with
      reference counting
  */
  BufferImpl(const BufferImpl<T, dimensions> &b) : buffer_base(b.read_only),
                                                   Allocation(b.Access),
                                                   Access(Allocation) {}


  /** Create a new sub-BufferImplImpl without allocation to have separate
      accessors later

      \todo To implement and deal with reference counting
  BufferImpl(BufferImpl<T, dimensions> b,
             index<dimensions> base_index,
             range<dimensions> sub_range)
  */

  /// \todo Allow CLHPP objects too?
  ///
  /*
  BufferImpl(cl_mem mem_object,
             queue from_queue,
             event available_event)
  */

  // Use BOOST_DISABLE_ASSERTS at some time to disable range checking

  /// Return an accessor of the required mode \param M
  template <access::mode mode,
            access::target target=access::global_buffer>
  AccessorImpl<T, dimensions, mode, target> get_access() {
    return { *this };
  }

};

/// @} to end the data Doxygen group

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
