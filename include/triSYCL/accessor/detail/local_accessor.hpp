#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_LOCAL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_LOCAL_ACCESSOR_HPP

/** \file The OpenCL SYCL local accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif
#include <boost/multi_array.hpp>

#include "triSYCL/access.hpp"
#include "triSYCL/command_group/detail/task.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"

namespace trisycl {

class handler;

namespace detail {

// Forward declaration of detail::accessor to declare the specialization
template <typename T,
          int Dimensions,
          access::mode Mode,
          access::target Target>
class accessor;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The local accessor specialization abstracts the way local memory
    is allocated to a kernel to be shared between work-items of the
    same work-group.

    \todo Use the access::mode
*/
template <typename T,
          int Dimensions,
          access::mode Mode>
class accessor<T, Dimensions, Mode, access::target::local> :
    public detail::debug<accessor<T,
                                  Dimensions,
                                  Mode,
                                  access::target::local>> {

  /// The implementation is a multi_array_ref wrapper
  using array_type = boost::multi_array_ref<T, Dimensions>;

  // The same type but writable
  // \todo Only if T is non const actually
  using writable_array_type =
    typename std::remove_const<array_type>::type;

  /** The way the buffer is really accessed

      Use a mutable member because the accessor needs to be captured
      by value in the lambda which is then read-only. This is to avoid
      the user to use mutable lambda or have a lot of const_cast as
      previously done in this implementation
   */
  mutable writable_array_type array;

  /** The allocation on the host for the local accessor

      Note that this is uninitialized memory, as stated in SYCL
      specification.
  */
  mutable T *allocation = nullptr;

public:

  /** \todo in the specification: store the dimension for user request

      \todo Use another name, such as from C++17 committee discussions.
   */
  static constexpr auto dimensionality = Dimensions;

  /** \todo in the specification: store the types for user request as STL
      or C++AMP */
  using value_type = T;
  using element = T;
  using reference = typename array_type::reference;
  using const_reference = typename array_type::const_reference;

  /** Inherit the iterator types from the implementation

      \todo Add iterators to accessors in the specification
  */
  using iterator = typename array_type::iterator;
  using const_iterator = typename array_type::const_iterator;
  using reverse_iterator = typename array_type::reverse_iterator;
  using const_reverse_iterator =
    typename array_type::const_reverse_iterator;


  /** Construct a device accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template param
  */
  accessor(const range<Dimensions> &allocation_size,
           handler &command_group_handler) :
    array { allocate_accessor(allocation_size) } {}


  // Deallocate the memory
  ~accessor() {
    deallocate_accessor();
  }


  /** Return a range object representing the size of the buffer in
      terms of number of elements in each dimension as passed to the
      constructor

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_range() const {
    /* Interpret the shape which is a pointer to the first element as an
       array of Dimensions elements so that the range<Dimensions>
       constructor is happy with this collection

       \todo Add also a constructor in range<> to accept a const
       std::size_t *?
    */
    return range<Dimensions> {
      *(const std::size_t (*)[Dimensions])(array.shape())
        };
  }


  /** Returns the total number of elements behind the accessor

      Equal to get_range()[0] * ... * get_range()[Dimensions-1].

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_count() const {
    return array.num_elements();
  }


  /** Returns the size of the underlying buffer storage in bytes

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_size() const {
    return get_count()*sizeof(value_type);
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  reference operator[](std::size_t index) {
    return array[index];
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  reference operator[](std::size_t index) const {
    return array[index];
  }


  /// To use the accessor with [id<>]
  auto &operator[](id<dimensionality> index) {
    return array(index);
  }


  /// To use the accessor with [id<>]
  auto &operator[](id<dimensionality> index) const {
    return array(index);
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> index) {
    return (*this)[index.get()];
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> index) const {
    return (*this)[index.get()];
  }


  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> index) {
    return (*this)[index.get_global()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> index) const {
    return (*this)[index.get_global()];
  }


  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add in the specification
  */
  reference operator*() {
    return *array.data();
  }


  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add in the specification?

      \todo Add the concept of 0-dim buffer and accessor for scalar
      and use an implicit conversion to value_type reference to access
      the value with the accessor?
  */
  reference operator*() const {
    return *array.data();
  }


  /** Test if the accessor has a read access right

      \todo Strangely, it is not really constexpr because it is not a
      static method...

      \todo to move in the access::mode enum class and add to the
      specification ?
  */
  constexpr bool is_read_access() const {
    return Mode == access::mode::read
      || Mode == access::mode::read_write
      || Mode == access::mode::discard_read_write;
  }


  /** Test if the accessor has a write access right

      \todo Strangely, it is not really constexpr because it is not a
      static method...

      \todo to move in the access::mode enum class and add to the
      specification ?
  */
  constexpr bool is_write_access() const {
    return Mode == access::mode::write
      || Mode == access::mode::read_write
      || Mode == access::mode::discard_write
      || Mode == access::mode::discard_read_write;
  }


  /** Forward all the iterator functions to the implementation

      \todo Add these functions to the specification

      \todo The fact that the lambda capture make a const copy of the
      accessor is not yet elegantly managed... The issue is that
      begin()/end() dispatch is made according to the accessor
      constness and not from the array member constness...

      \todo try to solve it by using some enable_if on array
      constness?

      \todo The issue is that the end may not be known if it is
      implemented by a raw OpenCL cl_mem... So only provide on the
      device the iterators related to the start? Actually the accessor
      needs to know a part of the shape to have the multidimentional
      addressing. So this only require a size_t more...

      \todo Factor out these in a template helper

      \todo Do we need this in detail::accessor too or only in accessor?
  */


  // iterator begin() { return array.begin(); }
  iterator begin() const {
    return const_cast<writable_array_type &>(array).begin();
  }


  // iterator end() { return array.end(); }
  iterator end() const {
    return const_cast<writable_array_type &>(array).end();
  }


  // const_iterator begin() const { return array.begin(); }


  // const_iterator end() const { return array.end(); }


  const_iterator cbegin() const { return array.begin(); }


  const_iterator cend() const { return array.end(); }


  // reverse_iterator rbegin() { return array.rbegin(); }
  reverse_iterator rbegin() const {
    return const_cast<writable_array_type &>(array).rbegin();
  }


  // reverse_iterator rend() { return array.rend(); }
  reverse_iterator rend() const {
    return const_cast<writable_array_type &>(array).rend();
  }


  // const_reverse_iterator rbegin() const { return array.rbegin(); }


  // const_reverse_iterator rend() const { return array.rend(); }


  const_reverse_iterator crbegin() const { return array.rbegin(); }


  const_reverse_iterator crend() const { return array.rend(); }

private:

  /// Allocate uninitialized buffer memory
  auto allocate_accessor(const range<Dimensions> &r) {
    auto count = r.size();
    // Allocate uninitialized memory
    allocation = std::allocator<value_type>{}.allocate(count);
    return boost::multi_array_ref<value_type, Dimensions> { allocation, r };
  }


  /// Deallocate accessor memory
  void deallocate_accessor() {
    std::allocator<value_type>{}.deallocate(allocation, array.num_elements());
  }


  // The following function are used from handler
  friend handler;


};

/// @} End the data Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_DETAIL_LOCAL_ACCESSOR_HPP
