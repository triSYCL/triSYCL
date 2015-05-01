#ifndef TRISYCL_SYCL_GROUP_HPP
#define TRISYCL_SYCL_GROUP_HPP

/** \file The OpenCL SYCL nd_item<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/id.hpp"
#include "CL/sycl/nd_range.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A group index used in a parallel_for_workitem to specify a work_group
 */
template <std::size_t dims = 1>
struct group {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = dims;

private:

  /// Keep a reference on the nd_range to serve potential query on it
  nd_range<dims> ndr;
  /// The coordinate of the group item
  id<dims> group_id;

public:

  /** Create a group from an nd_range<> with a 0 id<>

      \todo This should be private
  */
  group(const nd_range<dims> &ndr) : ndr { ndr } {}


  /** Create a group from an nd_range<> with a 0 id<>

      \todo This should be private
  */
  group(const nd_range<dims> &ndr, const id<dims> &i) :
    ndr { ndr }, group_id { i } {}


  /// Get the group identifier for this work_group
  id<dims> get_group_id() const { return group_id; }


  /// Get the local range for this work_group
  range<dims> get_local_range() const { return ndr.get_local_range(); }


  /// Get the local range for this work_group
  range<dims> get_global_range() const { return ndr.get_global_range(); }


  /// Get the offset of the NDRange
  id<dims> get_offset() const { return ndr.get_offset(); }


  /// \todo Also provide this access to the current nd_range
  nd_range<dims> get_nd_range() const { return ndr; }


  /** Return the group coordinate in the given dimension

      \todo In this implementation it is not const because the group<> is
      written in the parallel_for iterators. To fix according to the
      specification
   */
  auto &operator[](int dimension) {
    return group_id[dimension];
  }


  /// Return the group coordinate in the given dimension
  std::size_t get(int dimension) const {
    return group_id[dimension];
  }

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

#endif // TRISYCL_SYCL_GROUP_HPP
