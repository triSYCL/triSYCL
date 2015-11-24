#ifndef TRISYCL_SYCL_PIPE_HPP
#define TRISYCL_SYCL_PIPE_HPP

/** \file The OpenCL SYCL pipe<>

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "CL/sycl/access.hpp"
#include "CL/sycl/accessor.hpp"
#include "CL/sycl/handler.hpp"
#include "CL/sycl/pipe/detail/pipe.hpp"

namespace cl {
namespace sycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL pipe

    
*/
template <typename T>
struct pipe {
  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;

  std::shared_ptr<detail::pipe<T>> implementation;

  pipe(std::size_t capacity)
    : implementation { new detail::pipe<T> { capacity } } { }


  /** Get an accessor to the pipe with the required mode

      \param Mode is the requested access mode

      \param Target is the type of pipe access required

      \param[in] command_group_handler is the command group handler in
      which the kernel is to be executed
  */
  template <access::mode Mode,
            access::target Target = access::pipe>
  accessor<value_type, 1, Mode, Target>
  get_access(handler &command_group_handler) {
    static_assert(Target == access::pipe
                  || Target == access::blocking_pipe,
                  "get_access(handler) with pipes can only deal with "
                  "access::pipe or access::blocking_pipe");
    return { *this->implementation, command_group_handler };
  }


  /** Return the maximum number of elements that can fit in the pipe
   */
  std::size_t capacity() const {
    return implementation->capacity();
    }

};

/// @} End the execution Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_HPP
