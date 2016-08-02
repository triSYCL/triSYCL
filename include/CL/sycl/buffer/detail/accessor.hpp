#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP

/** \file The OpenCL SYCL buffer accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include <boost/multi_array.hpp>

#include "CL/sycl/access.hpp"
#include "CL/sycl/detail/debug.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"

namespace cl {
namespace sycl {

class handler;

namespace detail {

// Forward declaration of detail::buffer for use in accessor
template <typename T, std::size_t Dimensions> class buffer;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The buffer accessor abstracts the way buffer data are accessed
    inside a kernel in a multidimensional variable length array way.

    This implementation relies on boost::multi_array to provide this
    nice syntax and behaviour.

    Right now the aim of this class is just to access to the buffer in
    a read-write mode, even if capturing the multi_array_ref from a
    lambda make it const (since in examples we have lambda with [=]
    without mutable lambda).

    \todo Use the access::mode
*/
template <typename T,
          std::size_t Dimensions,
          access::mode Mode,
          access::target Target /* = access::global_buffer */>
class accessor : public detail::debug<accessor<T,
                                               Dimensions,
                                               Mode,
                                               Target>> {
  /// Keep a reference to the accessed buffer
  std::shared_ptr<detail::buffer<T, Dimensions>> buf;

  /// The implementation is a multi_array_ref wrapper
  using array_view_type = boost::multi_array_ref<T, Dimensions>;

  // The same type but writable
  using writable_array_view_type =
    typename std::remove_const<array_view_type>::type;

  /** The way the buffer is really accessed

      Use a mutable member because the accessor needs to be captured
      by value in the lambda which is then read-only. This is to avoid
      the user to use mutable lambda or have a lot of const_cast as
      previously done in this implementation
   */
  mutable array_view_type array;

public:

  /** \todo in the specification: store the dimension for user request

      \todo Use another name, such as from C++17 committee discussions.
   */
  static constexpr auto dimensionality = Dimensions;

  /** \todo in the specification: store the types for user request as STL
      or C++AMP */
  using value_type = T;
  using element = T;
  using reference = typename array_view_type::reference;
  using const_reference = typename array_view_type::const_reference;

  /** Inherit the iterator types from the implementation

      \todo Add iterators to accessors in the specification
  */
  using iterator = typename array_view_type::iterator;
  using const_iterator = typename array_view_type::const_iterator;
  using reverse_iterator = typename array_view_type::reverse_iterator;
  using const_reverse_iterator =
    typename array_view_type::const_reverse_iterator;


  /** Construct a host accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(std::shared_ptr<detail::buffer<T, Dimensions>> target_buffer) :
    buf { target_buffer }, array { target_buffer->access } {
    TRISYCL_DUMP_T("Create a host accessor write = " << is_write_access());
    static_assert(Target == access::target::host_buffer,
                  "without a handler, access target should be host_buffer");
    /* The host needs to wait for all the producers of the buffer to
       have finished */
    buf->wait();
  }


  /** Construct a device accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(std::shared_ptr<detail::buffer<T, Dimensions>> target_buffer,
           handler &command_group_handler) :
    buf { target_buffer }, array { target_buffer->access } {
    TRISYCL_DUMP_T("Create a kernel accessor write = " << is_write_access());
    static_assert(Target == access::target::global_buffer
                  || Target == access::target::constant_buffer,
                  "access target should be global_buffer or constant_buffer "
                  "when a handler is used");
    // Register the buffer to the task dependencies
    buffer_add_to_task(buf, &command_group_handler, is_write_access());
  }


  /// Returns the size of the underlying buffer in number of elements.
  std::size_t get_size() const {
    return array.num_elements();
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  reference operator[](std::size_t index) {
    return array[index];
  }


  /// To use the accessor in with [id<>]
  auto &operator[](id<dimensionality> index) {
    return array(index);
  }


  /// To use the accessor in with [id<>]
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


  /// Get the buffer used to create the accessor
  detail::buffer<T, Dimensions> &get_buffer() {
    return *buf;
  }


  /// Test if the accessor as a write access right
  constexpr bool is_write_access() const {
    /** \todo to move in the access::mode enum class and add to the
        specification ? */
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
  */


  // iterator begin() { return array.begin(); }
  iterator begin() const {
    return const_cast<writable_array_view_type &>(array).begin();
  }


  // iterator end() { return array.end(); }
  iterator end() const {
    return const_cast<writable_array_view_type &>(array).end();
  }


  // const_iterator begin() const { return array.begin(); }


  // const_iterator end() const { return array.end(); }


  const_iterator cbegin() const { return array.begin(); }


  const_iterator cend() const { return array.end(); }


  // reverse_iterator rbegin() { return array.rbegin(); }
  reverse_iterator rbegin() const {
    return const_cast<writable_array_view_type &>(array).rbegin();
  }


  // reverse_iterator rend() { return array.rend(); }
  reverse_iterator rend() const {
    return const_cast<writable_array_view_type &>(array).rend();
  }


  // const_reverse_iterator rbegin() const { return array.rbegin(); }


  // const_reverse_iterator rend() const { return array.rend(); }


  const_reverse_iterator crbegin() const { return array.rbegin(); }


  const_reverse_iterator crend() const { return array.rend(); }

};

/// @} End the data Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP
