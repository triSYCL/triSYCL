#ifndef TRISYCL_SYCL_EXCEPTION_HPP
#define TRISYCL_SYCL_EXCEPTION_HPP

/** \file The OpenCL SYCL exception

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <exception>

namespace trisycl {

/** \addtogroup error_handling Error handling
    @{
*/


/** A shared pointer to an exception as in C++ specification

    \todo Do we need this instead of reusing directly the one from C++11?
*/
using exception_ptr = std::exception_ptr;


/** Exception list to store several exceptions

    \todo Do we need to define it in SYCL or can we rely on plain C++17 one?
*/
struct exception_list : std::vector<exception_ptr> {
  using std::vector<exception_ptr>::vector;
};

using async_handler = function_class<void, exception_list>;


/// Encapsulate a SYCL error information
class exception {

  /// The error message to return
  string_class message;

public:

  /// Construct an exception with a message for internal use
  exception(const string_class &message) : message { message } {}

  /// Returns a descriptive string for the error, if available
  string_class what() const {
    return message;
  }


  /** Returns the context that caused the error

      Returns nullptr if not a buffer error.

      \todo Cannot return nullptr. Use optional? Use a specific exception type?
  */
  //context get_context()

};


/// Returns the OpenCL error code encapsulated in the exception
class cl_exception : public exception {

#ifdef TRISYCL_OPENCL
  /// The OpenCL error code to return

  cl_int cl_code;

public:

  /**  Construct an exception with a message and OpenCL error code for
       internal use */
  cl_exception(const string_class &message, cl_int cl_code)
    : exception { message }, cl_code { cl_code } {}

  // thrown as a result of an OpenCL API error code
  cl_int get_cl_code() const {
    return cl_code;
  }
#endif

};


/// An error stored in an exception_list for asynchronous errors
struct async_exception : exception {
  using exception::exception;
};


class runtime_error : public exception {
  using exception::exception;
};


/// Error that occurred before or while enqueuing the SYCL kernel
class kernel_error : public runtime_error {
  using runtime_error::runtime_error;
};


/// Error regarding the trisycl::accessor objects defined
class accessor_error : public runtime_error {
  using runtime_error::runtime_error;
};


/// Error regarding the trisycl::nd_range specified for the SYCL kernel
class nd_range_error : public runtime_error {
  using runtime_error::runtime_error;
};


/// Error regarding associated trisycl::event objects
class event_error : public runtime_error {
  using runtime_error::runtime_error;
};


/** Error regarding parameters to the SYCL kernel, it may apply to any
    captured parameters to the kernel lambda
*/
class invalid_parameter_error : public runtime_error {
  using runtime_error::runtime_error;
};


/// The SYCL device will trigger this exception on error
class device_error : public exception {
  using exception::exception;
};


/// Error while compiling the SYCL kernel to a SYCL device
class compile_program_error : public device_error {
  using device_error::device_error;
};


/// Error while linking the SYCL kernel to a SYCL device
class link_program_error : public device_error {
  using device_error::device_error;
};


/// Error regarding any memory objects being used inside the kernel
class invalid_object_error : public device_error {
  using device_error::device_error;
};


/// Error on memory allocation on the SYCL device for a SYCL kernel
class memory_allocation_error : public device_error {
  using device_error::device_error;
};


/// A failing pipe error will trigger this exception on error
class pipe_error : public runtime_error {
  using runtime_error::runtime_error;
};


/// The SYCL platform will trigger this exception on error
class platform_error : public device_error {
  using device_error::device_error;
};


/** The SYCL runtime will trigger this error if there is an error when
    profiling info is enabled
*/
class profiling_error : public device_error {
  using device_error::device_error;
};


/** Exception thrown when an optional feature or extension is used in
    a kernel but its not available on the device the SYCL kernel is
    being enqueued on
*/
class feature_not_supported : public device_error {
  using device_error::device_error;
};


/** Exception for an OpenCL operation requested in a non OpenCL area

    \todo Add to the specification

    \todo Clean implementation

    \todo Exceptions are named error in C++
*/
class non_cl_error : public runtime_error {
  using runtime_error::runtime_error;
};


/// @} End the error_handling Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_EXCEPTION_HPP
