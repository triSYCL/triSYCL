#ifndef TRISYCL_SYCL_STATIC_PIPE_HPP
#define TRISYCL_SYCL_STATIC_PIPE_HPP

/** \file The OpenCL SYCL static-scoped pipe equivalent to an OpenCL
    program-scoped pipe

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <memory>

#include "triSYCL/access.hpp"
#include "triSYCL/accessor.hpp"
#include "triSYCL/handler.hpp"
#include "triSYCL/pipe/detail/pipe.hpp"

namespace trisycl {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A SYCL static-scoped pipe equivalent to an OpenCL program-scoped
    pipe

    Implement a FIFO-style object that can be used through accessors
    to send some objects T from the input to the output.

    Compared to a normal pipe, a static_pipe takes a constexpr size
    and is expected to be declared in a compile-unit static context so
    the compiler can generate everything at compile time.

    This is useful to generate a fixed and optimized hardware
    implementation on FPGA for example, where the interconnection
    graph can be also inferred at compile time.

    It is not directly mapped to the OpenCL program-scoped pipe
    because in SYCL there is not this concept of separated
    program. But the SYCL device compiler is expected to generate some
    OpenCL program(s) with program-scoped pipes when a SYCL
    static-scoped pipe is used. These details are implementation
    defined.
*/
template <typename T, std::size_t Capacity>
class static_pipe
    /* Use the underlying pipe implementation that can be shared in
       the SYCL model */
  : public detail::shared_ptr_implementation<static_pipe<T, Capacity>,
                                             detail::pipe<T>>,
    detail::debug<static_pipe<T, Capacity>> {

  // The type encapsulating the implementation
  using implementation_t = typename static_pipe::shared_ptr_implementation;

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  // Allows the comparison operation to access the implementation
  friend implementation_t;

public:

  /// The STL-like types
  using value_type = T;


  /// Construct a static-scoped pipe able to store up to Capacity T objects
  static_pipe()
    : implementation_t { new detail::pipe<T> { Capacity } } { }


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


  /** Get an accessor to the pipe with the required mode, outside of a
      normal kernel, for example in host code

      \param Mode is the requested access mode

      \param Target is the type of pipe access required
  */
  template <access::mode Mode,
            access::target Target = access::target::pipe>
  accessor<value_type, 1, Mode, Target>
  get_access() {
    static_assert(Target == access::target::pipe
                  || Target == access::target::blocking_pipe,
                  "get_access(handler) with pipes can only deal with "
                  "access::pipe or access::blocking_pipe");
    return { implementation };
  }


  /** Return the maximum number of elements that can fit in the pipe

      This is a constexpr since the capacity is in the type.
  */
  std::size_t constexpr capacity() const {
    return Capacity;
    }

};

/// @} End the execution Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_STATIC_PIPE_HPP
