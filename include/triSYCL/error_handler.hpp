#ifndef TRISYCL_SYCL_ERROR_HANDLER_HPP
#define TRISYCL_SYCL_ERROR_HANDLER_HPP

/** \file The OpenCL SYCL error_handler

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include "triSYCL/exception.hpp"

namespace trisycl {

/** \addtogroup error_handling Error handling
    @{
*/

/// \todo Refactor when updating to latest specification
namespace trisycl {
  // Create a default error handler to be used when nothing is specified
  struct default_error_handler;
}


/** User supplied error handler to call a user-provided function when an
    error happens from a SYCL object that was constructed with this error
    handler
*/
struct error_handler {
  /** The method to define to be called in the case of an error

      \todo Add "virtual void" to the specification
  */
  virtual void report_error(exception &error) = 0;

  /** Add a default_handler to be used by default

      \todo add this concept to the specification?
  */
  static trisycl::default_error_handler default_handler;

  // Virtual to call the real destructor
  virtual ~error_handler() = 0;
};


namespace trisycl {

  struct default_error_handler : error_handler {

    void report_error(exception &) override {
    }
  };
}

  // \todo finish initialization
  //error_handler::default_handler = nullptr;


/// @} End the error_handling Doxygen group

}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_ERROR_HANDLER_HPP
