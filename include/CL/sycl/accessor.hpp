#ifndef TRISYCL_SYCL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_HPP

/** \file The OpenCL SYCL accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor/detail/accessor_traits.hpp"
#include "CL/sycl/accessor/detail/local_accessor.hpp"
#include "CL/sycl/buffer/detail/buffer_accessor_shepherd.hpp"
#include "CL/sycl/detail/shared_ptr_implementation.hpp"
#include "CL/sycl/id.hpp"
#include "CL/sycl/item.hpp"
#include "CL/sycl/nd_item.hpp"
#include "CL/sycl/pipe_reservation.hpp"
#include "CL/sycl/pipe/detail/pipe_accessor.hpp"

namespace cl::sycl {

template <typename T, int Dimensions, typename Allocator>
class buffer;
template <typename T>
class pipe;
class handler;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way buffer or pipe data are accessed
    inside a kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename DataType,
          int Dimensions,
          access::mode AccessMode,
          access::target Target = access::target::global_buffer>
class accessor :
    public detail::buffer_accessor_view<DataType
                                        , Dimensions
                                        , AccessMode
                                        , Target> {

 private:

  using accessor_view = typename detail::buffer_accessor_view<DataType
                                                              , Dimensions
                                                              , AccessMode
                                                              , Target>;

  /* Still specify all the template parameters since otherwise it will
     require a definition of the accessor which is still an incomplete
     type at this point */
  using shepherd = typename detail::buffer_accessor_shepherd<accessor
                                                             , DataType
                                                             , Dimensions
                                                             , AccessMode
                                                             , Target>;

 public:

  // Make the implementation member directly accessible in this class
  using accessor_view::buffer_accessor_view;

  /** Construct a buffer accessor from a buffer using a command group
      handler object from the command group scope

      Constructor only available for global_buffer or constant_buffer
      target.

      access_target defines the form of access being obtained.

      \todo Add template allocator type in all the accessor
      constructors in the specification or just use a more opaque
      Buffer type?

      \todo fix specification where access mode should be target
      instead
  */
  template <typename Allocator>
  accessor(buffer<DataType, Dimensions, Allocator> &target_buffer,
           handler &command_group_handler)
    : accessor_view {
        target_buffer.implementation->implementation->get_access()
      }
  {
    auto s =
      std::make_shared<shepherd>(target_buffer.implementation->implementation,
                                 command_group_handler);
    static_assert(Target == access::target::global_buffer
                  || Target == access::target::constant_buffer,
                  "access target should be global_buffer or constant_buffer "
                  "when a handler is used");
    // Now the accessor shepherd is created, register it
    s->register_accessor();
  }


  /** Construct a buffer accessor from a buffer

      Constructor only available for host_buffer target.

      access_target defines the form of access being obtained.
  */
  template <typename Allocator>
  accessor(buffer<DataType, Dimensions, Allocator> &target_buffer)
    : accessor_view {
        target_buffer.implementation->implementation->get_access()
      }
  {
    static_assert(Target == access::target::host_buffer,
                  "without a handler, access target should be host_buffer");
    auto s =
      std::make_shared<shepherd>(target_buffer.implementation->implementation);
    // \todo Do we need to extend the life of shepherd further?
    // Probably to fix with https://github.com/triSYCL/triSYCL/issues/192
  }

  /** Construct a buffer accessor from a buffer given a specific range for
      access permissions and an offset that provides the starting point
      for the access range using a command group handler object from the
      command group scope

      This accessor limits the processing of the buffer to the [offset,
      offset+range[ for every dimension. Any other parts of the buffer
      will be unaffected.

      Constructor only available for access modes global_buffer, and
      constant_buffer (see Table "Buffer accessor constructors").
      access_target defines the form of access being obtained.

      This accessor is recommended for discard-write and discard read
      write access modes, when the unaffected parts of the processing
      should be retained.
  */
  template <typename Allocator>
  accessor(buffer<DataType, Dimensions, Allocator> &target_buffer,
           handler &command_group_handler,
           const range<Dimensions> &offset,
           const range<Dimensions> &range) {
    TRISYCL_UNIMPL;
  }


  /** Construct an accessor of dimension Dimensions with elements of type
      DataType using the passed range to specify the size in each
      dimension

      It needs as a parameter a command group handler object from the
      command group scope. Constructor only available if AccessMode is
      local, see Table 3.25.
  */
  accessor(const range<Dimensions> &allocation_size,
           handler &command_group_handler)
#if 0
    : implementation_t { new detail::accessor<DataType,
                                              Dimensions,
                                              AccessMode,
                                              access::target::local> {
      allocation_size, command_group_handler
        }
  }
  {
    static_assert(Target == access::target::local,
                  "This accessor constructor requires "
                  "access target be local");
  }
#endif
{}


};

#if 0
/** The pipe accessor abstracts the way pipe data are accessed inside
    a kernel

    A specialization for an non-blocking pipe
*/
template <typename DataType,
          access::mode AccessMode>
class accessor<DataType, 1, AccessMode, access::target::pipe> :
    public detail::pipe_accessor<DataType, AccessMode, access::target::pipe> {
public:

  using accessor_detail =
    detail::pipe_accessor<DataType, AccessMode, access::target::pipe>;
  // Inherit of the constructors to have accessor constructor from detail
  using accessor_detail::accessor_detail;

  /** Construct a pipe accessor from a pipe using a command group
      handler object from the command group scope

      access_target defines the form of access being obtained.
  */
  accessor(pipe<DataType> &p, handler &command_group_handler)
    : accessor_detail { p.implementation, command_group_handler } { }

  /// Make a reservation inside the pipe
  pipe_reservation<accessor> reserve(std::size_t size) const {
    return accessor_detail::reserve(size);
  }


  /// Get the underlying pipe implementation
  auto &get_pipe_detail() {
    return accessor_detail::get_pipe_detail();
  }

};


/** The pipe accessor abstracts the way pipe data are accessed inside
    a kernel

    A specialization for a blocking pipe
*/
template <typename DataType,
          access::mode AccessMode>
class accessor<DataType, 1, AccessMode, access::target::blocking_pipe> :
    public detail::pipe_accessor<DataType,
                                 AccessMode,
                                 access::target::blocking_pipe> {
public:

  using accessor_detail =
    detail::pipe_accessor<DataType, AccessMode, access::target::blocking_pipe>;
  // Inherit of the constructors to have accessor constructor from detail
  using accessor_detail::accessor_detail;

  /** Construct a pipe accessor from a pipe using a command group
      handler object from the command group scope

      access_target defines the form of access being obtained.
  */
  accessor(pipe<DataType> &p, handler &command_group_handler)
    : accessor_detail { p.implementation, command_group_handler } { }


  /** Construct a pipe accessor from a pipe outside of a normal
      kernel, for example in host code
  */
  accessor(pipe<DataType> &p)
    : accessor_detail { p.implementation } { }


  /// Make a reservation inside the pipe
  pipe_reservation<accessor> reserve(std::size_t size) const {
    return accessor_detail::reserve(size);
  }


  /// Get the underlying pipe implementation
  auto &get_pipe_detail() {
    return accessor_detail::get_pipe_detail();
  }

};
#endif

/** Top-level function to break circular dependencies on the the types
    to get the pipe implementation */
template <typename Accessor>
static inline auto &get_pipe_detail(Accessor &a) {
  return a.get_pipe_detail();
  }

/// @} End the data Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_HPP
