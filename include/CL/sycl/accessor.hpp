#ifndef TRISYCL_SYCL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_HPP

/** \file The OpenCL SYCL accessor<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor/detail/accessor.hpp"


namespace cl {
namespace sycl {

template <typename T, std::size_t Dimensions, typename Allocator> struct buffer;

class handler;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way buffer data are accessed inside a
    kernel in a multidimensional variable length array way.

    \todo Implement it for images according so section 3.3.4.5
*/
template <typename DataType,
          std::size_t Dimensions,
          access::mode AccessMode,
          access::target Target = access::global_buffer>
struct accessor : detail::accessor<DataType, Dimensions, AccessMode, Target> {
  /// \todo in the specification: store the dimension for user request
  static constexpr auto dimensionality = Dimensions;
  using value_type = DataType;
  using reference = value_type&;
  using const_reference = const value_type&;

  // Inherit of the constructors to have accessor constructor from detail
  using detail::accessor<DataType, Dimensions, AccessMode, Target>::accessor;

  /** Construct a buffer accessor from a buffer using a command group
      handler object from the command group scope

      Constructor only available for access modes global_buffer,
      host_buffer, constant_buffer (see Table 3.25).

      access_target defines the form of access being obtained. See Table
      3.26.
  */
  template <typename Allocator>
  accessor(buffer<DataType, Dimensions, Allocator> &target_buffer,
           handler &command_group_handler) :
    detail::accessor<DataType, Dimensions, AccessMode, Target> ( *target_buffer.implementation ) {}


  /** Construct a buffer accessor from a buffer given a specific range for
      access permissions and an offset that provides the starting point
      for the access range using a command group handler object from the
      command group scope

      This accessor limits the processing of the buffer to the [offset,
      offset+range[ for every dimension. Any other parts of the buffer
      will be unaffected.

      Constructor only available for access modes global_buffer,
      host_buffer or constant_buffer (see Table 3.25). access_target
      defines the form of access being obtained (see Table 3.26).

      This accessor is recommended for discard-write and discard read
      write access modes, when the unaffected parts of the processing
      should be retained.
  */
  template <typename Allocator>
  accessor(buffer<DataType, Dimensions, Allocator> &target_buffer,
           handler &command_group_handler,
           range<Dimensions> offset,
           range<Dimensions> range) {
    detail::unimplemented();
  }


  /** Construct an accessor of dimensions Dimensions with elements of type
      DataType using the passed range to specify the size in each
      dimension

      It needs as a parameter a command group handler object from the
      command group scope. Constructor only available if AccessMode is
      local, see Table 3.25.
  */
  accessor(range<Dimensions> allocation_size,
           handler &command_group_handler) {
    detail::unimplemented();
  }

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

#endif // TRISYCL_SYCL_ACCESSOR_HPP
