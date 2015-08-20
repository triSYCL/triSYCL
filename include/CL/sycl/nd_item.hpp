#ifndef TRISYCL_SYCL_ND_ITEM_HPP
#define TRISYCL_SYCL_ND_ITEM_HPP

/** \file The OpenCL SYCL nd_item<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/access.hpp"
#include "CL/sycl/detail/linear_id.hpp"
#include "CL/sycl/detail/unimplemented.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A SYCL nd_item stores information on a work-item within a work-group,
    with some more context such as the definition ranges.
*/
template <std::size_t dims = 1>
struct nd_item {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = dims;

private:

  id<dims> global_index;
  /* This is a cached value since it can be computed from global_index and
     ND_range */
  id<dims> local_index;
  nd_range<dims> ND_range;

public:

  /** Create an empty nd_item<> from an nd_range<>

      \todo This is for the triSYCL implementation which is expected to
      call set_global() and set_local() later. This should be hidden to
      the user.
  */
  nd_item(nd_range<dims> ndr) : ND_range { ndr } {}


  /** Create a full nd_item

      \todo This is for validation purpose. Hide this to the programmer
      somehow
  */
  nd_item(id<dims> global_index,
          nd_range<dims> ndr) :
    global_index { global_index },
    // Compute the local index using the offset and the group size
    local_index { (global_index - ndr.get_offset())%id<dims> { ndr.get_local() } },
    ND_range { ndr }
  {}


  /** To be able to copy and assign nd_item, use default constructors too

      \todo Make most of them protected, reserved to implementation
  */
  nd_item() = default;


  /** Return the constituent global id representing the work-item's
      position in the global iteration space
  */
  id<dims> get_global() const { return global_index; }


  /** Return the constituent element of the global id representing the
      work-item's position in the global iteration space in the given
      dimension
  */
  size_t get_global(int dimension) const { return get_global()[dimension]; }


  /** Return the flattened id of the current work-item after subtracting
      the offset
  */
  size_t get_global_linear_id() const {
    return detail::linear_id(get_global_range(), get_global(), get_offset());
  }


  /** Return the constituent local id representing the work-item's
      position within the current work-group
  */
  id<dims> get_local() const { return local_index; }


  /** Return the constituent element of the local id representing the
      work-item's position within the current work-group in the given
      dimension
  */
  size_t get_local(int dimension) const { return get_local()[dimension]; }


  /** Return the flattened id of the current work-item within the current
      work-group
   */
  size_t get_local_linear_id() const {
    return detail::linear_id(get_local_range(), get_local());
  }


  /** Return the constituent group group representing the work-group's
      position within the overall nd_range
  */
  id<dims> get_group() const {
    /* Convert get_local_range() to an id<> to remove ambiguity into using
       implicit conversion either from range<> to id<> or the opposite */
    return get_global()/id<dims> { get_local_range() };
  }


  /** Return the constituent element of the group id representing the
      work-group;s position within the overall nd_range in the given
      dimension.
  */
  size_t get_group(int dimension) const {
    return get_group()[dimension];
  }


  /// Return the flattened id of the current work-group
  size_t get_group_linear_id() const {
    return detail::linear_id(get_num_groups(), get_group());
  }


  /// Return the number of groups in the nd_range
  id<dims> get_num_groups() const {
    return get_nd_range().get_group();
  }

  /// Return the number of groups for dimension in the nd_range
  size_t get_num_groups(int dimension) const {
     return get_num_groups()[dimension];
  }


  /// Return a range<> representing the dimensions of the nd_range<>
  range<dims> get_global_range() const {
    return get_nd_range().get_global();
  }


  /// Return a range<> representing the dimensions of the current work-group
  range<dims> get_local_range() const {
    return get_nd_range().get_local();
  }


  /** Return an id<> representing the n-dimensional offset provided to the
      constructor of the nd_range<> and that is added by the runtime to the
      global-ID of each work-item
  */
  id<dims> get_offset() const { return get_nd_range().get_offset(); }


  /// Return the nd_range<> of the current execution
  nd_range<dims> get_nd_range() const { return ND_range; }


  /** Execute a barrier with memory ordering on the local address space,
      global address space or both based on the value of flag

      The current work-item will wait at the barrier until all work-items
      in the current work-group have reached the barrier.

      In addition, the barrier performs a fence operation ensuring that all
      memory accesses in the specified address space issued before the
      barrier complete before those issued after the barrier
  */
  void barrier(access::fence_space flag =
               access::fence_space::global_and_local) const {
#if defined(_OPENMP) && !defined(TRISYCL_NO_BARRIER)
    /* Use OpenMP barrier in the implementation with 1 OpenMP thread per
       work-item of the work-group */
#pragma omp barrier
#else
    // \todo To be implemented efficiently otherwise
    detail::unimplemented();
#endif
  }


  // For the triSYCL implementation, need to set the local index
  void set_local(id<dims> Index) { local_index = Index; }


  // For the triSYCL implementation, need to set the global index
  void set_global(id<dims> Index) { global_index = Index; }

};

/// @} End the parallelism Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ND_ITEM_HPP
