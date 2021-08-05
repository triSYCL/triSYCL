#ifndef TRISYCL_SYCL_ACCESSOR_DETAIL_LOCAL_ACCESSOR_HPP
#define TRISYCL_SYCL_ACCESSOR_DETAIL_LOCAL_ACCESSOR_HPP

/** \file The SYCL local accessor<> detail behind the scene for a CPU
    implementation

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "triSYCL/access.hpp"
#include "triSYCL/buffer/detail/accessor.hpp"
#include "triSYCL/detail/debug.hpp"
#include "triSYCL/range.hpp"

namespace trisycl {

class handler;

namespace detail {

// Forward declaration of detail::accessor to declare the specialization
template <typename T, int Dimensions, access::mode Mode, access::target Target>
class accessor;

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The local accessor specialization abstracts the way local memory
    is allocated to a kernel to be shared between work-items of the
    same work-group.

    Since this a pure library implementation, implement it as a
    host_accessor using its own local buffer.

    \todo Implement a real local accessor handling several work-group
    running in parallel
*/
template <typename T, int Dimensions, access::mode Mode>
class accessor<T, Dimensions, Mode, access::target::local>
    : public accessor<T, Dimensions, Mode, access::target::host_buffer>
    , public detail::debug<
          accessor<T, Dimensions, Mode, access::target::local>> {
  using hb = accessor<T, Dimensions, Mode, access::target::host_buffer>;

  // Storage used by this local accessor instance
  std::shared_ptr<buffer<T, Dimensions>> buf;

 public:
  /// Construct a local accessor of the right size
  accessor(const range<Dimensions>& allocation_size,
           handler& command_group_handler)
      : buf { std::make_shared<buffer<T, Dimensions>>(allocation_size) } {
    this->set_buffer(buf);
    this->set_access(buf->access);
  }
};

/// @} End the data Doxygen group

} // namespace detail
} // namespace trisycl

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ACCESSOR_DETAIL_LOCAL_ACCESSOR_HPP
