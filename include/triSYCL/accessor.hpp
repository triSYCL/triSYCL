#ifndef TRISYCL_SYCL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_HPP

/** \file The OpenCL SYCL accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "triSYCL/access.hpp"
#include "triSYCL/accessor/detail/local_accessor.hpp"
#include "triSYCL/buffer/detail/accessor.hpp"
#include "triSYCL/detail/container_element_aspect.hpp"
#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/id.hpp"
#include "triSYCL/item.hpp"
#include "triSYCL/nd_item.hpp"
#include "triSYCL/pipe_reservation.hpp"
#include "triSYCL/pipe/detail/pipe_accessor.hpp"

namespace trisycl {

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
    public detail::shared_ptr_implementation<accessor<DataType,
                                                      Dimensions,
                                                      AccessMode,
                                                      Target>,
                                             detail::accessor<DataType,
                                                              Dimensions,
                                                              AccessMode,
                                                              Target>>,
    public detail::container_element_aspect<DataType> {

 public:

  /// \todo in the specification: store the dimension for user request
  static constexpr auto dimensionality = Dimensions;

 private:

  using accessor_detail = typename detail::accessor<DataType,
                                                    Dimensions,
                                                    AccessMode,
                                                    Target>;

  // The type encapsulating the implementation
  using implementation_t = typename accessor::shared_ptr_implementation;

  // Allows the comparison operation to access the implementation
  friend implementation_t;

 public:

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

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
           handler &command_group_handler) : implementation_t {
    new detail::accessor<DataType, Dimensions, AccessMode, Target> {
      target_buffer.implementation->implementation, command_group_handler }
  } {
    static_assert(Target == access::target::global_buffer
                  || Target == access::target::constant_buffer,
                  "access target should be global_buffer or constant_buffer "
                  "when a handler is used");
    // Now the implementation is created, register it
    implementation->register_accessor();
  }


  /** Construct a buffer accessor from a buffer

      Constructor only available for host_buffer target.

      access_target defines the form of access being obtained.
  */
  template <typename Allocator>
  accessor(buffer<DataType, Dimensions, Allocator> &target_buffer)
    : implementation_t {
    new detail::accessor<DataType, Dimensions, AccessMode, Target> {
      target_buffer.implementation->implementation }
  } {
    static_assert(Target == access::target::host_buffer,
                  "without a handler, access target should be host_buffer");
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
    return implementation->get_range();
  }


  /** Returns the total number of elements behind the accessor

      Equal to get_range()[0] * ... * get_range()[Dimensions-1].

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_count() const {
    return implementation->get_count();
  }


  /** Returns the size of the underlying buffer storage in bytes

      \todo It is incompatible with buffer get_size() in the spec

      \todo Move on
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15564 and
      https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14404
  */
  auto get_size() const {
    return implementation->get_size();
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  typename accessor_detail::reference operator[](std::size_t index) {
//#ifdef TRISYCL_DEVICE
//#else
    return (*implementation)[index];
//#endif
  }


  /** Use the accessor with integers à la [][][]

      Use array_view_type::reference instead of auto& because it does not
      work in some dimensions.
   */
  typename accessor_detail::reference operator[](std::size_t index) const {
    return (*implementation)[index];
  }


  /// To use the accessor with [id<>]
  auto &operator[](id<dimensionality> index) {
    return (*implementation)[index];
  }


  /// To use the accessor with [id<>]
  auto &operator[](id<dimensionality> index) const {
    return (*implementation)[index];
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> index) {
    return (*this)[index.get_id()];
  }


  /// To use an accessor with [item<>]
  auto &operator[](item<dimensionality> index) const {
    return (*this)[index.get_id()];
  }


  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> index) {
    return (*this)[index.get_global_id()];
  }

  /** To use an accessor with an [nd_item<>]

      \todo Add in the specification because used by HPC-GPU slide 22
  */
  auto &operator[](nd_item<dimensionality> index) const {
    return (*this)[index.get_global_id()];
  }


    /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add in the specification
  */
  typename accessor_detail::reference operator*() {
    return **implementation;
  }


  /** Get the first element of the accessor

      Useful with an accessor on a scalar for example.

      \todo Add in the specification?

      \todo Add the concept of 0-dim buffer and accessor for scalar
      and use an implicit conversion to value_type reference to access
      the value with the accessor?
  */
  typename accessor_detail::reference operator*() const {
    return **implementation;
  }


  /** Get the pointer to the start of the data

      \todo Should it be named data() instead? */
  auto
  get_pointer() const {
    return implementation->get_pointer();
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
  typename accessor_detail::iterator begin() const {
    return implementation->begin();
  }


  // iterator end() { return array.end(); }
  typename accessor_detail::iterator end() const {
    return implementation->end();
  }


  // const_iterator begin() const { return implementation->begin(); }


  // const_iterator end() const { return implementation->end(); }


  typename accessor_detail::const_iterator cbegin() const {
    return implementation->cbegin();
  }


  typename accessor_detail::const_iterator cend() const {
    return implementation->cend();
  }


  typename accessor_detail::reverse_iterator rbegin() const {
    return implementation->rbegin();
  };


  typename accessor_detail::reverse_iterator rend() const {
    return implementation->rend();
  }


  // const_reverse_iterator rbegin() const { return array.rbegin(); }


  // const_reverse_iterator rend() const { return array.rend(); }


  typename accessor_detail::const_reverse_iterator crbegin() const {
    return implementation->rbegin();
  }


  typename accessor_detail::const_reverse_iterator crend() const {
    return implementation->rend();
  }

};


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
    public detail::pipe_accessor<DataType, AccessMode, access::target::blocking_pipe> {
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
