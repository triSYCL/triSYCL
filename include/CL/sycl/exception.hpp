#ifndef TRISYCL_SYCL_EXCEPTION_HPP
#define TRISYCL_SYCL_EXCEPTION_HPP

/** \file The OpenCL SYCL exception

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "CL/sycl/buffer.hpp"
#include "CL/sycl/image.hpp"

namespace cl {
namespace sycl {

class queue;

/** \addtogroup error_handling Error handling
    @{
*/

using async_handler = function_class<int/*cl::sycl::exception_list*/>;

/**
   Encapsulate a SYCL error information
*/
struct exception {
#ifdef TRISYCL_OPENCL
  /** Get the OpenCL error code

      \returns 0 if not an OpenCL error

      \todo to be implemented
  */
  cl_int get_cl_code() { assert(0); }


  /** Get the SYCL-specific error code

      \returns 0 if not a SYCL-specific error

      \todo to be implemented

      \todo use something else instead of cl_int to be usable without
      OpenCL
  */
  cl_int get_sycl_code() { assert(0); }
#endif

  /** Get the queue that caused the error

      \return nullptr if not a queue error

      \todo Update specification to replace 0 by nullptr
  */
  queue *get_queue() { assert(0); }


  /** Get the buffer that caused the error

      \returns nullptr if not a buffer error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented

      \todo How to get the real buffer type? Update: has been removed in
      new specification
  */
  template <typename T, int dimensions, typename Allocator>
  buffer<T, dimensions, Allocator> *get_buffer() {
    assert(0); }


  /** Get the image that caused the error

      \returns nullptr if not a image error

      \todo Update specification to replace 0 by nullptr and add the
      templated buffer

      \todo to be implemented
  */
  template <std::size_t dimensions> image<dimensions> *get_image() { assert(0); }
};

/// @} End the error_handling Doxygen group

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_EXCEPTION_HPP
