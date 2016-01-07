#ifndef TRISYCL_SYCL_PROGRAM_PIPE_HPP
#define TRISYCL_SYCL_PROGRAM_PIPE_HPP

/** \file The OpenCL SYCL program-scoped pipe

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

/** A SYCL program-scoped pipe

    Implement a FIFO-style object that can be used through accessors
    to send some objects T from the input to the output
*/
template <typename T, std::size_t Capacity>
struct program_pipe {
  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;

  /// The implementation is defined elsewhere
  std::shared_ptr<detail::pipe<T>> implementation;


  /// Construct a program-scoped pipe able to store up to Capacity T objects
  program_pipe()
    : implementation { new detail::pipe<T> { Capacity } } { }


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

      This is a constexpr since the capacity is in the type of a
      program-scoped pipe.
  */
  std::size_t constexpr capacity() const {
    return Capacity;
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

#endif // TRISYCL_SYCL_PROGRAM_PIPE_HPP
