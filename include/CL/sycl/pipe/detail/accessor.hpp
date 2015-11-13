#ifndef TRISYCL_SYCL_PIPE_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_ACCESSOR_HPP

/** \file The OpenCL SYCL pipe accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/access.hpp"
#include "CL/sycl/detail/debug.hpp"
#include "CL/sycl/pipe/detail/pipe.hpp"

namespace cl {
namespace sycl {

class handler;

namespace detail {

// Forward declaration of detail::accessor to declare the specialization
template <typename T,
          std::size_t Dimensions,
          access::mode Mode,
          access::target Target /* = access::global_buffer */>
struct accessor;
/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way pipe data are accessed inside a
    kernel
*/
template <typename T,
          access::mode AccessMode>
struct accessor<T, 1, AccessMode, access::pipe> :
    public detail::debug<accessor<T, 1, AccessMode, access::pipe>> {

  detail::pipe<T> &implementation;

  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;

  /** Construct a pipe accessor from an existing pipe
   */
  accessor(detail::pipe<T> &p, handler &command_group_handler) :
    implementation { p } {
    //    TRISYCL_DUMP_T("Create a kernel pipe accessor write = "
    //                 << is_write_access());
  }


  /** Try to write a value to the pipe

      \param[in] value is what we want to write

      \return true on success

      \todo provide a && version

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
   */
  bool write(const T &value) const {
    return implementation.write(value);
  }


  /** Try to read a value from the pipe

      \param[out] value is the reference to where to store what is
      read

      \return true on success

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
  */
  bool read(T &value) const {
    return implementation.read(value);
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

#endif // TRISYCL_SYCL_PIPE_DETAIL_ACCESSOR_HPP
