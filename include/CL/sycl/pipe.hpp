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

    Implement a FIFO-style object that can be used through accessors
    to send some objects T from the input to the output
*/
template <typename T>
class pipe
    /* Use the underlying pipe implementation that can be shared in
       the SYCL model */
  : public detail::shared_ptr_implementation<pipe<T>, detail::pipe<T>>,
    detail::debug<pipe<T>> {

  // The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<pipe<T>, detail::pipe<T>>;

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

public:

  /// The STL-like types
  /* Since a pipe element cannot be directly addressed without
     accessor, only define value_type here */
  using value_type = T;


  /// Construct a pipe able to store up to capacity T objects
  pipe(std::size_t capacity)
    : implementation_t { new detail::pipe<T> { capacity } } { }


  /** Get an accessor to the pipe with the required mode

      \param Mode is the requested access mode

      \param Target is the type of pipe access required

      \param[in] command_group_handler is the command group handler in
      which the kernel is to be executed
  */
  template <access::mode Mode,
            access::target Target = access::target::pipe>
  accessor<value_type, 1, Mode, Target>
  get_access(handler &command_group_handler) {
    static_assert(Target == access::target::pipe
                  || Target == access::target::blocking_pipe,
                  "get_access(handler) with pipes can only deal with "
                  "access::pipe or access::blocking_pipe");
    return { implementation, command_group_handler };
  }


  /// Return the maximum number of elements that can fit in the pipe
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
