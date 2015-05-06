#ifndef TRISYCL_SYCL_ITEM_HPP
#define TRISYCL_SYCL_ITEM_HPP

/** \file The OpenCL SYCL item<>

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

      \todo what is the meaning of this constructor for a programmer?
  */
  item(range<dims> global_size,
       id<dims> global_index,
       id<dims> offset = id<dims>()) :
    global_range { global_size },
    global_index { global_index },
    offset { offset }
  {}


  /** To be able to copy and assign item, use default constructors too

      \todo Make most of them protected, reserved to implementation
  */
  item() = default;

  /// Get the whole global id coordinate
  id<dims> get_global_id() const { return global_index; }


  /// Return the global coordinate in the given dimension
  size_t get(int dimension) const { return global_index[dimension]; }


  /// Return an l-value of the global coordinate in the given dimension
  auto &operator[](int dimension) { return global_index[dimension]; }


  /// Get the global range where this item dwells in
  range<dims> get_range() const { return global_range; }


  /// Get the offset associated with the item context
  id<dims> get_offset() const { return offset; }


  /** Return the linearized ID in the item's range

      Computed as the flatted ID after the offset is subtracted.
  */
  size_t get_linear_id() const {
    size_t linear_id = 0;
    for (int i = dims - 1; i >= 0; --i)
      linear_id = linear_id*get_range()[i]
        + get_global_id()[i] - get_offset()[i];

    return linear_id;
  }


  /** For the implementation, need to set the global index

      \todo Move to private and add friends
  */
  void set_global(id<dims> Index) { global_index = Index; }


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
