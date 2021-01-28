#ifndef TRISYCL_SYCL_VENDOR_TRISYCL_PIPE_COUT_HPP
#define TRISYCL_SYCL_VENDOR_TRISYCL_PIPE_COUT_HPP

/** \file The triSYCL data-flow pipe extension to send data to std::cout

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <iostream>

namespace trisycl::vendor::trisycl::pipe {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** triSYCL extension for SYCL pipe object sending data to std::cout

    Experiment with a new pipe extension to send data to std::cout
    with a simplified pipe extension like with:

    \code
    sycl::vendor::trisycl::pipe::cout::write("salut !\n");
    sycl::vendor::trisycl::pipe::cout::stream() << "hello " << 42 << std::endl;
    \endcode
*/
class cout {

public:

  /// Provide the usual stream interface to std::cout
  static auto& stream() {
    return std::cout;
  }

  /// Send some value to std::cout
  template <typename T>
  static void write(const T &value) {
    stream() << value;
  }

};


/// @} End the execution Doxygen group

}

#endif // TRISYCL_SYCL_VENDOR_TRISYCL_PIPE_COUT_HPP
