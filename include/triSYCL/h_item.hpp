#ifndef TRISYCL_SYCL_H_ITEM_HPP
#define TRISYCL_SYCL_H_ITEM_HPP

/** \file The OpenCL SYCL h_item<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "triSYCL/access.hpp"
#include "triSYCL/detail/linear_id.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_range.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** \c h_item<int dimensions> identifies an instance of a \c
    group::parallel_for_work_item function object executing at each
    point in a local \c range<int dimensions> passed to a \c
    parallel_for_work_item call or to the corresponding \c
    parallel_for_work_group call if no range is passed to the \c
    parallel_for_work_item call. It encapsulates enough information to
    identify the work-item’s local and global items according to the
    information given to \c parallel_for_work_group (physical ids) as
    well as the work-item’s logical local items in the flexible
    range. All returned items objects are offset-less. Instances of
    the \c h_item<int dimensions> class are not user-constructible and
    are passed by the runtime to each instance of the function object.
*/
template <int Dimensions = 1>
struct h_item : boost::equality_comparable<h_item<Dimensions>> {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = Dimensions;

private:

  id<Dimensions> global_index;
  /* This is a cached value since it can be computed from global_index and
     ND_range */
  id<Dimensions> local_index;
  nd_range<Dimensions> ND_range;

public:

  /** Create an empty nd_item<> from an nd_range<>

      \todo This is for the triSYCL implementation which is expected to
      call set_global() and set_local() later. This should be hidden to
      the user.
  */
  h_item(nd_range<Dimensions> ndr) : ND_range { ndr } {}


  /** Create a full nd_item

      \todo This is for validation purpose. Hide this to the programmer
      somehow
  */
  h_item(id<Dimensions> global_index,
          nd_range<Dimensions> ndr) :
    global_index { global_index },
    // Compute the local index using the offset and the group size
    local_index
      { (global_index - ndr.get_offset())%id<Dimensions> { ndr.get_local() } },
    ND_range { ndr }
  {}


  /** To be able to copy and assign nd_item, use default constructors too

      \todo Make most of them protected, reserved to implementation
  */
  h_item() = default;


  /** Return the constituent global id representing the work-item's
      position in the global iteration space
  */
  id<Dimensions> get_global_id() const { return global_index; }


  /** Return the constituent element of the global id representing the
      work-item's position in the global iteration space in the given
      dimension
  */
  size_t get_global_id(int dimension) const {
    return get_global_id()[dimension];
  }


  /** Return the flattened id of the current work-item after subtracting
      the offset
  */
  size_t get_global_linear_id() const {
    return detail::linear_id(get_global_range(), get_global_id(), get_offset());
  }


  /** Return the constituent local id representing the work-item's
      position within the current work-group
  */
  id<Dimensions> get_local_id() const { return local_index; }


  /** Return the constituent element of the local id representing the
      work-item's position within the current work-group in the given
      dimension
  */
  size_t get_local_id(int dimension) const { return get_local_id()[dimension]; }


  /** Return the flattened id of the current work-item within the current
      work-group
   */
  size_t get_local_linear_id() const {
    return detail::linear_id(get_local_range(), get_local_id());
  }


  /** Return the constituent group group representing the work-group's
      position within the overall nd_range
  */
  id<Dimensions> get_group() const {
    /* Convert get_local_range() to an id<> to remove ambiguity into using
       implicit conversion either from range<> to id<> or the opposite */
    return get_global_id()/id<Dimensions> { get_local_range() };
  }


  /** Return the constituent element of the group id representing the
      work-group's position within the overall nd_range in the given
      dimension.
  */
  size_t get_group(int dimension) const {
    return get_group()[dimension];
  }


  /// Return the flattened id of the current work-group
  size_t get_group_linear_id() const {
    return detail::linear_id(get_group_range(), get_group());
  }


  /// Return the number of groups in the nd_range
  id<Dimensions> get_group_range() const {
    return get_nd_range().get_group_range();
  }

  /// Return the number of groups for dimension in the nd_range
  size_t get_group_range(int dimension) const {
     return get_group_range()[dimension];
  }


  /// Return a range<> representing the dimensions of the nd_range<>
  range<Dimensions> get_global_range() const {
    return get_nd_range().get_global_range();
  }


  /// Return a range<> representing the dimensions of the current work-group
  range<Dimensions> get_local_range() const {
    return get_nd_range().get_local_range();
  }


  /** Return an id<> representing the n-dimensional offset provided to the
      constructor of the nd_range<> and that is added by the runtime to the
      global-ID of each work-item
  */
  id<Dimensions> get_offset() const { return get_nd_range().get_offset(); }


  /// Return the nd_range<> of the current execution
  nd_range<Dimensions> get_nd_range() const { return ND_range; }


  /** Allows projection down to an item

      \todo Add to the specification
  */
  item<Dimensions> get_item() const {
    return { get_global_range(), get_global_id(), get_offset() };
  }


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
    TRISYCL_UNIMPL;
#endif
  }


  // For the triSYCL implementation, need to set the local index
  void set_local(id<Dimensions> Index) { local_index = Index; }


  // For the triSYCL implementation, need to set the global index
  void set_global(id<Dimensions> Index) { global_index = Index; }

  /// comparison operators
  bool operator==(const h_item<Dimensions> &itemB) const {
    return (ND_range == itemB.ND_range &&
            global_index == itemB.global_index);
  }
};

/// @} End the parallelism Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_H_ITEM_HPP
