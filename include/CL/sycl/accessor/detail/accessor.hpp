#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP

/** \file The OpenCL SYCL accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

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
template <typename T, std::size_t Dimensions> struct buffer;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    This implementation rely on boost::multi_array to provides this nice
    syntax and behaviour.

    Right now the aim of this class is just to access to the buffer in a
    read-write mode, even if capturing the multi_array_ref from a lambda
    make it const (since in some example we have lambda with [=] and
    without mutable). The access::mode is not used yet.
*/
template <typename T,
          std::size_t Dimensions,
          access::mode Mode,
          access::target Target /* = access::global_buffer */>
struct accessor : public detail::debug<accessor<T,
                                                Dimensions,
                                                Mode,
                                                Target>> {
  detail::buffer<T, Dimensions> *buf;
  // The implementation is a multi_array_ref wrapper
  using array_view_type = boost::multi_array_ref<T, Dimensions>;
  // \todo Do we need this if we have a reference on buf?
  array_view_type array;

  // The same type but writable
  using writable_array_view_type =
    typename std::remove_const<array_view_type>::type;

  // \todo in the specification: store the dimension for user request
  static const auto dimensionality = Dimensions;
  // \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = T;
  using value_type = T;

  /** Construct a host accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(detail::buffer<T, Dimensions> &target_buffer) :
    buf { &target_buffer }, array { target_buffer.access } {
#if TRISYCL_ASYNC
    if (Target == access::target::host_buffer) {
      // A host accessor needs to be declared *outside* a command_group
      assert(current_task == nullptr);
      // Wait for the latest generation of the buffer before the host can use it
      buffer_base::wait(target_buffer);
    }
    else {
      // A host non-host accessor needs to be declared *inside* a command_group
      assert(current_task != nullptr);
      // Register the accessor to the task dependencies
      current_task->add(*this);
    }
#endif
  }


  /** Construct a device accessor from an existing buffer

      \todo fix the specification to rename target that shadows
      template parm
  */
  accessor(detail::buffer<T, Dimensions> &target_buffer,
           handler &command_group_handler) :
    buf { &target_buffer },
    array { target_buffer.access } {
#if TRISYCL_ASYNC
    if (Target == access::target::host_buffer) {
      // A host accessor needs to be declared *outside* a command_group
      assert(current_task == nullptr);
      // Wait for the latest generation of the buffer before the host can use it
      buffer_base::wait(target_buffer);
    }
    else {
      // A host non-host accessor needs to be declared *inside* a command_group
      assert(current_task != nullptr);
      // Register the accessor to the task dependencies
      current_task->add(*this);
    }
#endif
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  typename array_view_type::reference operator[](std::size_t index) {
    return array[index];
  }


  /// To use the accessor in with [id<>]
  auto &operator[](id<dimensionality> index) {
    return (const_cast<writable_array_view_type &>(array))(index);
  }


  /** To use the accessor in with [id<>]

      This is when we access to accessor[] that we override the const
      if any
  */
  auto &operator[](id<dimensionality> index) const {
    return (const_cast<writable_array_view_type &>(array))(index);
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
