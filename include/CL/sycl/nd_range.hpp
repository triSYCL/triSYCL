#ifndef TRISYCL_SYCL_ND_RANGE_HPP
#define TRISYCL_SYCL_ND_RANGE_HPP

/** \file The OpenCL SYCL nd_range<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/id.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A ND-range, made by a global and local range, to specify work-group
    and work-item organization.

    The local offset is used to translate the iteration space origin if
    needed.

    \todo add copy constructors in the specification
*/
template <std::size_t dims = 1>
struct nd_range {
  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = dims;

private:

  range<dimensionality> global_range;
  range<dimensionality> local_range;
  id<dimensionality> offset;

public:

  /** Construct a ND-range with all the details available in OpenCL

      By default use a zero offset, that is iterations start at 0
   */
  nd_range(range<dims> global_size,
           range<dims> local_size,
           id<dims> offset = {}) :
    global_range { global_size }, local_range { local_size }, offset { offset }
  { }


  /// Get the global iteration space range
  range<dims> get_global() const { return global_range; }


  /// Get the local part of the iteration space range
  range<dims> get_local() const { return local_range; }


  /// Get the range of work-groups needed to run this ND-range
  auto get_group() const {
    // \todo Assume that global_range is a multiple of local_range, element-wise
    return global_range/local_range;
  }


  /// \todo get_offset() is lacking in the specification
  id<dims> get_offset() const { return offset; }


  /// Display the value for debugging and validation purpose
  void display() const {
    global_range.display();
    local_range.display();
    offset.display();
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

#endif // TRISYCL_SYCL_ND_RANGE_HPP
