#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_ACCESSOR_HPP

/** \file The OpenCL SYCL accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include <boost/multi_array.hpp>

#include "CL/sycl/detail/debug.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"

namespace cl {
namespace sycl {
namespace trisycl {
// Forward declaration for use in accessor
template <typename T, std::size_t dimensions> struct BufferImpl;
}
namespace detail {


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
          std::size_t dimensions,
          access::mode mode,
          access::target target /* = access::global_buffer */>
struct AccessorImpl : public detail::debug<AccessorImpl<T,
                                                        dimensions,
                                                        mode,
                                                        target>> {
  trisycl::BufferImpl<T, dimensions> *Buffer;
  // The implementation is a multi_array_ref wrapper
  typedef boost::multi_array_ref<T, dimensions> ArrayViewType;
  // \todo Do we need this if we have a reference on Buffer?
  ArrayViewType Array;

  // The same type but writable
  typedef typename std::remove_const<ArrayViewType>::type WritableArrayViewType;

  // \todo in the specification: store the dimension for user request
  static const auto dimensionality = dimensions;
  // \todo in the specification: store the types for user request as STL
  // or C++AMP
  using element = T;
  using value_type = T;


  /// The only way to construct an AccessorImpl is from an existing buffer
  // \todo fix the specification to rename target that shadows template parm
  AccessorImpl(trisycl::BufferImpl<T, dimensions> &targetBuffer) :
    Buffer { &targetBuffer }, Array { targetBuffer.Access } {
#if TRISYCL_ASYNC
    if (target == access::target::host_buffer) {
      // A host accessor needs to be declared *outside* a command_group
      assert(CurrentTask == nullptr);
      // Wait for the latest generation of the buffer before the host can use it
      buffer_base::wait(targetBuffer);
    }
    else {
      // A host non-host accessor needs to be declared *inside* a command_group
      assert(CurrentTask != nullptr);
      // Register the accessor to the task dependencies
      CurrentTask->add(*this);
    }
#endif
  }


  /** Use the accessor in with integers à la [][][]

      Use ArrayViewType::reference instead of auto& because it does not
      work in some dimensions.
   */
  typename ArrayViewType::reference operator[](std::size_t Index) {
    return Array[Index];
  }


  /// To use the accessor in with [id<>]
  auto &operator[](id<dimensionality> Index) {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }


  /** To use the accessor in with [id<>]

      This is when we access to AccessorImpl[] that we override the const
      if any
  */
  auto &operator[](id<dimensionality> Index) const {
    return (const_cast<WritableArrayViewType &>(Array))(Index);
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> Index) {
    return (*this)[Index.get_global_id()];
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> Index) const {
    return (*this)[Index.get_global_id()];
  }


  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because use by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> Index) {
    return (*this)[Index.get_global_id()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because use by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> Index) const {
    return (*this)[Index.get_global_id()];
  }


  /// Get the buffer used to create the accessor
  trisycl::BufferImpl<T, dimensions> &getBuffer() {
    return *Buffer;
  }


  /// Test if the accessor as a write access right
  constexpr bool isWriteAccess() const {
    /** \todo to move in the access::mode enum class and add to the
        specification ? */
    return mode == access::mode::write
      || mode == access::mode::read_write
      || mode == access::mode::discard_write
      || mode == access::mode::discard_read_write;
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
