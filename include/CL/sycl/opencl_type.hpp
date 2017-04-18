#ifndef TRISYCL_SYCL_OPENCL_TYPE_HPP
#define TRISYCL_SYCL_OPENCL_TYPE_HPP

/** \file
    triSYCL wrapper for OpenCL types

    Joan DOT Thibault AT ens-rennes DOT fr

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

// Only provide OpenCL interoperability types when OpenCL is in use
#ifdef TRISYCL_OPENCL

#include <boost/compute.hpp>

namespace cl {
namespace sycl {

/** Implement OpenCL interoperability types

    \todo Implement all of them
*/

/** Wrapper of Boost::compute's cl_float3
 */
class cl_float3 {

  ::cl_float3 self;

public :

  cl_float3 () = default;


  cl_float3 (::cl_float3 self_) : self { self_ }
  {}


  cl_float3 (float x, float y, float z) : self { x, y, z }
  {}


  /* Return the first element of the vector
   */
  auto& x() {
    return self.s[0];
  }


  /* Return the second element of the vector
   */
  auto& y() {
    return self.s[1];
  }


  /* Return the third element of the vector
   */
  auto& z() {
    return self.s[2];
  }

};

}
}

#endif

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif //  TRISYCL_SYCL_OPENCL_TYPE_HPP
