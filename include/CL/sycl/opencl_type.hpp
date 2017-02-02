#ifndef TRISYCL_SYCL_OPENCL_TYPE_HPP
#define TRISYCL_SYCL_OPENCL_TYPE_HPP

/** \file
    triSYCL wrapper for openCL types
    Joan DOT Thibault AT ens-rennes DOT fr
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <boost/compute.hpp>

namespace cl {
namespace sycl {


/** Wrapper of Boost::compute's cl_float3
 */
class cl_float3 {


  ::cl_float3 yo; //yo: spannish word for I
  //inline auto operator[] = internal.operator[];
public :
  cl_float3 (::cl_float3 yo_) :
    yo { yo_ }
  {}

  cl_float3 (float x, float y, float z) :
    yo { x, y, z }
  {}

  auto& x() {return yo.s[0];}
  auto& y() {return yo.s[1];}
  auto& z() {return yo.s[2];}

};

}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif //  TRISYCL_SYCL_OPENCL_TYPE_HPP
