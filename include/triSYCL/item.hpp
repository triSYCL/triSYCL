#ifndef TRISYCL_SYCL_ITEM_HPP
#define TRISYCL_SYCL_ITEM_HPP

/** \file The OpenCL SYCL item<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "triSYCL/detail/linear_id.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

/** \addtogroup parallelism Expressing parallelism through kernels
    @{
*/

/** A SYCL item stores information on a work-item with some more context
    such as the definition range and offset.
*/
template <int Dimensions = 1, bool with_offset = true>
class item : boost::equality_comparable<item<Dimensions>> {

public:

  /// \todo add this Boost::multi_array or STL concept to the
  /// specification?
  static constexpr auto dimensionality = Dimensions;

private:

  range<Dimensions> global_range;
  id<Dimensions> global_index;
  id<Dimensions> offset;

public:

  /** Create an item from a local size and an optional offset

      This constructor is used by the triSYCL implementation and the
      non-regression testing.
  */
  item(range<Dimensions> global_size,
       id<Dimensions> global_index,
       id<Dimensions> offset = {}) :
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
  id<Dimensions> get_id() const { return global_index; }


  /** Return the requested dimension of the constituent id<> representing
      the work-item's position in the iteration space
  */
  size_t get_id(int dimension) const { return get_id()[dimension]; }


  /** Return the constituent id<> l-value representing the work-item's
      position in the iteration space in the given dimension
  */
  auto &operator[](int dimension) { return global_index[dimension]; }


  /** Returns a range<> representing the dimensions of the range of
      possible values of the item
  */
  range<Dimensions> get_range() const { return global_range; }

  /// Return the same value as get_range().get(dimension)
  size_t get_range(int dimension) const { return get_range().get(dimension); }

  /** Returns an id<> representing the n-dimensional offset provided to
      the parallel_for and that is added by the runtime to the global-ID
      of each work-item, if this item represents a global range

      For an item representing a local range of where no offset was passed
      this will always return an id of all 0 values.
  */
  id<Dimensions> get_offset() const {
    static_assert(with_offset == true, "get_offset only callable with has_offset = true");
    return offset;
  }


  /** Return the linearized ID in the item's range

      Computed as the flatted ID after the offset is subtracted.
  */
  size_t get_linear_id() const {
    return detail::linear_id(get_range(), get_id(), get_offset());
  }


  /** For the implementation, need to set the global index

      \todo Move to private and add friends
  */
  void set(id<Dimensions> Index) { global_index = Index; }

  /** Returns an item with same dimensions but offset set to 0 */
  operator item<Dimensions, true> () const {
    static_assert(with_offset == false, "get_offset only callable with has_offset = true");
    return { this->get_range(), this->get_id(), 0 };
  }

  /// Display the value for debugging and validation purpose
  void display() const {
    global_range.display();
    global_index.display();
    offset.display();
  }

  /// Comparison operators for item
  bool operator==(const item<Dimensions> &itemB) const {
    return (global_range == itemB.global_range &&
            global_index == itemB.global_index &&
            offset == itemB.offset);
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

#endif // TRISYCL_SYCL_ITEM_HPP
