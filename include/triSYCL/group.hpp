#ifndef TRISYCL_SYCL_GROUP_HPP
#define TRISYCL_SYCL_GROUP_HPP

/** \file The OpenCL SYCL group<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <functional>

#include "triSYCL/detail/linear_id.hpp"
#include "triSYCL/h_item.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/nd_range.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

template <int Dimensions = 1>
struct group;

namespace detail {

template <int Dimensions = 1, typename ParallelForFunctor>
void parallel_for_workitem_in_group(const group<Dimensions> &g,
                                    ParallelForFunctor f);

}

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A group index used in a parallel_for_workitem to specify a work_group
 */
template <int Dimensions>
struct group : boost::equality_comparable<group<Dimensions>> {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = Dimensions;

private:

  /// The coordinate of the group item
  id<Dimensions> group_id;

  /// Keep a reference on the nd_range to serve potential query on it
  nd_range<Dimensions> ndr;

public:

  /** Create a group from an nd_range<> with a 0 id<>

      \todo This should be private since it is only used by the triSYCL
      implementation
  */
  group(const nd_range<Dimensions> &ndr) : ndr { ndr } {}


  /** Create a group from an id and a nd_range<>

      \todo This should be private somehow, but it is used by the
      validation infrastructure
  */
  group(const id<Dimensions> &i, const nd_range<Dimensions> &ndr ) :
    group_id { i }, ndr { ndr } {}


  /** To be able to copy and assign group, use default constructors too

      \todo Make most of them protected, reserved to implementation
  */
  group() = default;


  /** Return an id representing the index of the group within the nd_range
      for every dimension
  */
  id<Dimensions> get_id() const { return group_id; }


  /// Return the index of the group in the given dimension
  size_t get_id(int dimension) const { return get_id()[dimension]; }


  /** Return the index of the group in the given dimension within the
      nd_range<>

      \todo In this implementation it is not const because the group<> is
      written in the parallel_for iterators. To fix according to the
      specification
  */
  auto &operator[](int dimension) {
    return group_id[dimension];
  }


  /** Return a range<> representing the dimensions of the current
      group

      This local range may have been provided by the programmer, or chosen
      by the runtime.

      \todo Fix this comment and the specification
  */
  range<Dimensions> get_group_range() const {
    return get_nd_range().get_group_range();
  }


  /// Return element dimension from the con stituent group range
  size_t get_group_range(int dimension) const {
    return get_group_range()[dimension];
  }


  /// Get the local range for this work_group
  range<Dimensions> get_global_range() const {
    return get_nd_range().get_global_range();
  }


  /// Return element dimension from the constituent global range
  size_t get_global_range(int dimension) const {
    return get_global_range()[dimension];
  }


  /** Get the local range for this work_group

      \todo Add to the specification
  */
  range<Dimensions> get_local_range() const {
    return get_nd_range().get_local_range();
  }


  /** Return element dimension from the constituent local range

      \todo Add to the specification
  */
  size_t get_local_range(int dimension) const {
    return get_local_range()[dimension];
  }


  /**  Get the offset of the NDRange

       \todo Add to the specification
  */
  id<Dimensions> get_offset() const { return get_nd_range().get_offset(); }


  /**  Get the offset of the NDRange

       \todo Add to the specification
  */
  size_t get_offset(int dimension) const { return get_offset()[dimension]; }


  /// \todo Also provide this access to the current nd_range
  nd_range<Dimensions> get_nd_range() const { return ndr; }


  /** Get a linearized version of the group ID

   */
  size_t get_linear_id() const {
    return detail::linear_id(get_group_range(), get_id());
  }


  /** Loop on the work-items inside a work-group
   */
  void parallel_for_work_item(std::function<void(h_item<dimensionality>)> f)
    const {
    detail::parallel_for_workitem_in_group(*this, f);
  }

  /* Comparison operators for group object.
   */
  bool operator==(const group &groupB) const {
    return (group_id == groupB.group_id &&
	    ndr == groupB.ndr);
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

#endif // TRISYCL_SYCL_GROUP_HPP
