#ifndef TRISYCL_SYCL_HALF_HPP
#define TRISYCL_SYCL_HALF_HPP

/** \file SYCL Half support - dummy
    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
 */

/* Dummy half implementation - this is enough to get the CTS to build. */
namespace trisycl {

class half {
public:
  half(int) {};

  bool operator>(const half &h1) { return false; };
};

}

#endif
