#ifndef TRISYCL_SYCL_PROGRAM_HPP
#define TRISYCL_SYCL_PROGRAM_HPP

/** \file The OpenCL SYCL program

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/shared_ptr_implementation.hpp"
#include "triSYCL/detail/unimplemented.hpp"
#include "triSYCL/kernel.hpp"
#include "triSYCL/program/detail/program.hpp"

namespace trisycl {

enum class program_state {
  none,
  compiled,
  linked
};

/** SYCL program

    \todo To be implemented

*/

class program
    /* Use the underlying kernel implementation that can be shared in
     the SYCL model */
  : public detail::shared_ptr_implementation<program, detail::program> {

  // The type encapsulating the implementation
  using implementation_t = typename program::shared_ptr_implementation;

  // The handler class uses the implementation
  friend class handler;

  // Allows the comparison operation to access the implementation
  friend implementation_t;

 public:

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  program() = delete;

#ifdef TRISYCL_OPENCL

  program(const context &context, cl_program p) {}

  cl_program get() const {
    return implementation->get();
  }

  template <typename kernelT>
  bool has_kernel() const;

  bool has_kernel(string_class kernelName) const;

  template <typename kernelT>
  void compile_with_kernel_type(string_class compileOptions = "")
  {

  }

  template <typename kernelT>
  void build_with_kernel_type(string_class buildOptions = "")
  {
  }

  template <typename kernelT>
  ::trisycl::kernel get_kernel() const;

  kernel get_kernel(string_class kernelName) const {
    return NULL;
  }
#endif
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PROGRAM_HPP
