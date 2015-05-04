#ifndef TRISYCL_SYCL_ND_ITEM_HPP
#define TRISYCL_SYCL_ND_ITEM_HPP

/** \file The OpenCL SYCL nd_item<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/access.hpp"
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
  id<dims> local_index;
  nd_range<dims> ND_range;

public:

  /** Create an nd_item from a local size and local size

      \todo what is the meaning of this constructor for a programmer?
  */
  nd_item(range<dims> global_size, range<dims> local_size) :
    ND_range { global_size, local_size } {}


  /** \todo a constructor from a nd_range too in the specification if the
      previous one has a meaning?
   */
  nd_item(nd_range<dims> ndr) : ND_range { ndr } {}


  /** Create a full nd_item

      \todo this is for validation purpose. Hide this to the programmer
      somehow
  */
  nd_item(id<dims> global_index,
          id<dims> local_index,
          nd_range<dims> ndr) :
    global_index { global_index }, local_index { local_index }, ND_range { ndr } {}


  /// Get the whole global id coordinate
  id<dims> get_global_id() const { return global_index; }


  /// Get the whole local id coordinate (which is respective to the
  /// work-group)
  id<dims> get_local_id() const { return local_index; }


  /// Get the whole group id coordinate
  id<dims> get_group_id() const { return get_global_id()/get_local_range(); }


  /// Return the global coordinate in the given dimension
  auto get_global_id(int dimension) const { return get_global_id()[dimension]; }


  /// Return the local coordinate (that is in the work-group) in the given
  /// dimension
  auto get_local_id(int dimension) const { return get_local_id()[dimension]; }


  /// Get the whole group id coordinate in the given dimension
  id<dims> get_group_id(int dimension) const {
    return get_group_id()[dimension];
  }


  /// Get the global range where this nd_item dwells in
  range<dims> get_global_range() const { return ND_range.get_global_range(); }


  /// Get the local range (the dimension of the work-group) for this nd_item
  range<dims> get_local_range() const { return ND_range.get_local_range(); }


  /// Get the offset of the NDRange
  id<dims> get_offset() const { return ND_range.get_offset(); }


  /// Get the NDRange for this nd_item
  nd_range<dims> get_nd_range() const { return ND_range; }


  /** Executes a barrier with memory ordering on the local address space,
      global address space or both based on the value of flag. The current
      work- item will wait at the barrier until all work-items in the
      current work-group have reached the barrier.  In addition the
      barrier performs a fence operation ensuring that all memory accesses
      in the specified address space issued before the barrier complete
      before those issued after the barrier.

      \todo To be implemented
  */
  void barrier(access::address_space flag) const {}


  // For the implementation, need to set the local index
  void set_local(id<dims> Index) { local_index = Index; }


  // For the implementation, need to set the global index
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
