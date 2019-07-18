#ifndef TRISYCL_SYCL_PROGRAM_DETAIL_PROGRAM_HPP
#define TRISYCL_SYCL_PROGRAM_DETAIL_PROGRAM_HPP

/** \file The OpenCL SYCL program

    Dave Airlie

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/
#ifdef TRISYCL_OPENCL
#include <boost/compute.hpp>
#endif

#include "triSYCL/detail/debug.hpp"
#include "triSYCL/detail/unimplemented.hpp"

namespace trisycl::detail {

class program : detail::debug<detail::program> {

   public:

#ifdef TRISYCL_OPENCL
  /** Return the OpenCL kernel object for this kernel

      Retains a reference to the returned cl_kernel object. Caller
      should release it when finished.
  */
  virtual cl_program get() const = 0;
#endif

  virtual ~program() {}
};

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_DETAIL_PROGRAM_PROGRAM_HPP


