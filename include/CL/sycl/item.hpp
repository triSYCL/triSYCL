#ifndef TRISYCL_SYCL_ITEM_HPP
#define TRISYCL_SYCL_ITEM_HPP

/** \file The OpenCL SYCL item<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/detail/linear_id.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/range.hpp"

namespace cl {
namespace sycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A SYCL item stores information on a work-item with some more context
    such as the definition range and offset.
*/
template <std::size_t dims = 1>
class item {

public:

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = dims;

private:

  range<dims> global_range;
  id<dims> global_index;
  id<dims> offset;

public:

  /** Create an item from a local size and an optional offset

      This constructor is used by the triSYCL implementation and the
      non-regression testing.
  */
  item(range<dims> global_size,
       id<dims> global_index,
       id<dims> offset = {}) :
    global_range { global_size },
    global_index { global_index },
    offset { offset }
  {}


  /** To be able to copy and assign item, use default constructors too

      \todo Make most of them protected, reserved to implementation
  */
  item() = default;


  /** Return the constituent local or global id<> representing the
      work-item's position in the iteration space
  */
  id<dims> get() const { return global_index; }


  /** Return the requested dimension of the constituent id<> representing
      the work-item's position in the iteration space
  */
  size_t get(int dimension) const { return get()[dimension]; }


  /** Return the constituent id<> l-value representing the work-item's
      position in the iteration space in the given dimension
  */
  auto &operator[](int dimension) { return global_index[dimension]; }


  /** Returns a range<> representing the dimensions of the range of
      possible values of the item
  */
  range<dims> get_range() const { return global_range; }


  /** Returns an id<> representing the n-dimensional offset provided to
      the parallel_for and that is added by the runtime to the global-ID
      of each work-item, if this item represents a global range

      For an item representing a local range of where no offset was passed
      this will always return an id of all 0 values.
  */
  id<dims> get_offset() const { return offset; }


  /** Return the linearized ID in the item's range

      Computed as the flatted ID after the offset is subtracted.
  */
  size_t get_linear_id() const {
    return detail::linear_id(get_range(), get(), get_offset());
  }


  /** For the implementation, need to set the global index

      \todo Move to private and add friends
  */
  void set(id<dims> Index) { global_index = Index; }


  /// Display the value for debugging and validation purpose
  void display() const {
    global_range.display();
    global_index.display();
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

#endif // TRISYCL_SYCL_ITEM_HPP
