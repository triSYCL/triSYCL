#ifndef TRISYCL_SYCL_ND_RANGE_HPP
#define TRISYCL_SYCL_ND_RANGE_HPP

/** \file The OpenCL SYCL nd_range<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "triSYCL/id.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A ND-range, made by a global and local range, to specify work-group
    and work-item organization.

    The local offset is used to translate the iteration space origin if
    needed.

    \todo add copy constructors in the specification
*/
template <int Dimensions = 1>
struct nd_range : boost::equality_comparable<nd_range<Dimensions>> {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = Dimensions;

private:

  range<dimensionality> global_range;
  range<dimensionality> local_range;
  id<dimensionality> offset;

public:

  /** Construct a ND-range with all the details available in OpenCL

      By default use a zero offset, that is iterations start at 0
   */
  nd_range(range<Dimensions> global_size,
           range<Dimensions> local_size,
           id<Dimensions> offset = {}) :
    global_range { global_size }, local_range { local_size }, offset { offset }
  { }


  /// Get the global iteration space range
  range<Dimensions> get_global_range() const { return global_range; }


  /// Get the local part of the iteration space range
  range<Dimensions> get_local_range() const { return local_range; }


  /// Get the range of work-groups needed to run this ND-range
  auto get_group_range() const {
    /* This is basically global_range/local_range, round up to the
       next integer, in case the global range is not a multiple of the
       local range. Note this is a motivating example to build a range
       from a scalar with a broadcasting constructor. */
    return (global_range + local_range - range<Dimensions>{ 1 })/local_range;
  }


  /// \todo get_offset() is lacking in the specification
  id<Dimensions> get_offset() const { return offset; }


  /// Display the value for debugging and validation purpose
  void display() const {
    global_range.display();
    local_range.display();
    offset.display();
  }

  /// Comparison operators for nd_range.
  bool operator==(const nd_range &nd_rangeB) const {
    return (global_range == nd_rangeB.global_range &&
	    offset == nd_rangeB.offset);
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

#endif // TRISYCL_SYCL_ND_RANGE_HPP
